#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "parking.h" // Makefile: -Iinclude

// --- Terminal helpers ---
static int term_cols(void)
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return 0;
    return (int)w.ws_col;
}

static int term_rows(void)
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return 0;
    return (int)w.ws_row;
}

static void term_clear_all(void)
{
    printf("\033[2J\033[H\033[3J");
    fflush(stdout);
}

static void term_enter_alt_screen(void)
{
    printf("\033[?1049h"); // alternate screen buffer
    printf("\033[?7l");    // disable line wrap
    printf("\033[?25l");   // hide cursor
    fflush(stdout);
}

static void term_leave_alt_screen(void)
{
    printf("\033[?25h");   // show cursor
    printf("\033[?7h");    // enable line wrap
    printf("\033[?1049l"); // back to normal screen
    fflush(stdout);
}

// --- Raw input (une seule fois) ---
static struct termios g_oldterm;
static int g_term_inited = 0;
static int g_oldflags = 0;

static void input_enable_raw(void)
{
    struct termios t;

    if (tcgetattr(STDIN_FILENO, &g_oldterm) == -1)
        return;
    t = g_oldterm;

    t.c_lflag &= ~(ICANON | ECHO); // pas canonique, pas d'écho
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    g_oldflags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, g_oldflags | O_NONBLOCK);

    g_term_inited = 1;
}

static void input_disable_raw(void)
{
    if (!g_term_inited)
        return;

    tcsetattr(STDIN_FILENO, TCSANOW, &g_oldterm);
    fcntl(STDIN_FILENO, F_SETFL, g_oldflags);
    g_term_inited = 0;
}

static void cleanup_and_exit(int code)
{
    input_disable_raw();
    term_leave_alt_screen();
    exit(code);
}

// --- Map dimensions (ASCII only) ---
static void get_map_dimensions_ascii(const char *path, int *out_cols, int *out_rows)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        *out_cols = 0;
        *out_rows = 0;
        return;
    }

    char line[4096];
    int max_cols = 0;
    int rows = 0;

    while (fgets(line, sizeof(line), f))
    {
        int len = 0;
        while (line[len] && line[len] != '\n' && line[len] != '\r')
            len++;

        if (len > max_cols)
            max_cols = len;
        rows++;
    }

    fclose(f);
    *out_cols = max_cols;
    *out_rows = rows;
}

// Lecture non-bloquante d'une touche
char key_pressed(void)
{
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n == 1)
        return (char)c;
    return 0;
}

static void redraw_everything(const char *map_path, int selected_spot_index)
{
    term_clear_all();
    display_static_map(map_path);
    init_spots_from_map(map_path);
    draw_all_spots(selected_spot_index);
    fflush(stdout);
}

int main(void)
{
    const char *map_path = "assets/parking_map_ascii.txt";

    // Taille minimale requise (calculée depuis la map ASCII)
    int map_cols = 0, map_rows = 0;
    get_map_dimensions_ascii(map_path, &map_cols, &map_rows);

    int REQ_COLS = map_cols + 2; // marge
    int REQ_ROWS = map_rows + 2;

    // on limite la hauteur demandée pour éviter de bloquer sur certains terminaux
    if (REQ_ROWS > 55)
        REQ_ROWS = 55;

    term_enter_alt_screen();
    term_clear_all();
    input_enable_raw();

    int cols = term_cols();
    int rows = term_rows();

    if (cols < REQ_COLS || rows < REQ_ROWS)
    {
        term_clear_all();
        printf("⚠️ Terminal trop petit pour afficher correctement.\n");
        printf("Min requis: %dx%d | Actuel: %dx%d\n\n", REQ_COLS, REQ_ROWS, cols, rows);
        printf("➡️ Agrandis la fenetre puis appuie sur 'r' pour redessiner.\n");
        printf("Appuie sur 'e' pour quitter.\n");
        fflush(stdout);
        // on NE quitte PAS
    }

    int selected_spot_index = 0;
    redraw_everything(map_path, selected_spot_index);

    int last_cols = cols;
    int last_rows = rows;

    char key = 0;
    while (key != 'e')
    {
        cols = term_cols();
        rows = term_rows();

        // Resize -> pause + 'r' pour redraw
        if (cols != last_cols || rows != last_rows)
        {
            term_clear_all();
            printf("⚠️ Redimensionnement detecte.\n");
            printf("Min: %dx%d | Actuel: %dx%d\n\n", REQ_COLS, REQ_ROWS, cols, rows);
            printf("Agrandis si besoin, puis appuie sur 'r' pour redessiner.\n");
            printf("'e' pour quitter.\n");
            fflush(stdout);

            char k = 0;
            while (1)
            {
                cols = term_cols();
                rows = term_rows();
                k = key_pressed();

                if (k == 'e')
                    cleanup_and_exit(0);
                if (k == 'r' && cols >= REQ_COLS)
                    break;

                usleep(50000);
            }

            last_cols = cols;
            last_rows = rows;
            redraw_everything(map_path, selected_spot_index);
        }

        key = key_pressed();

        if (key != 0)
        {
            int old_selection = selected_spot_index;
            int selection_changed = 0;

            switch (key)
            {
            case 'q':
                if (selected_spot_index > 0)
                {
                    selected_spot_index--;
                    selection_changed = 1;
                }
                break;

            case 'd':
                if (selected_spot_index < TOTAL_SPOTS - 1)
                {
                    selected_spot_index++;
                    selection_changed = 1;
                }
                break;

            case 'z':
                if (selected_spot_index > 0)
                {
                    selected_spot_index--;
                    selection_changed = 1;
                }
                break;

            case 's':
                if (selected_spot_index < TOTAL_SPOTS - 1)
                {
                    selected_spot_index++;
                    selection_changed = 1;
                }
                break;

            case ' ':
                all_spots[selected_spot_index].is_occupied =
                    !all_spots[selected_spot_index].is_occupied;
                draw_spot(all_spots[selected_spot_index], 1);
                break;
            }

            if (selection_changed)
            {
                draw_spot(all_spots[old_selection], 0);
                draw_spot(all_spots[selected_spot_index], 1);
            }

            fflush(stdout);
        }

        usleep(50000);
    }

    cleanup_and_exit(0);
    return 0;
}