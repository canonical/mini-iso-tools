/*
 * Copyright 2022-2023 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This program presents a menu of installation ISOs that we can possibly
 * chain-boot to.  It uses JSON information obtained from SimpleStreams to
 * provide the list of ISOs with a friendly label.
 *
 * The menu is styled to have an appearance that is as close to Subiquity as
 * possible.
 *
 * Input is a file similar to
 * http://cdimage.ubuntu.com/streams/v1/com.ubuntu.cdimage.daily:ubuntu-server.json
 *
 * The chosen ISO is output in a format friendly for the /bin/sh source
 * built-in - sample output:
 *
 * MEDIA_URL="https://releases.ubuntu.com/kinetic/ubuntu-22.10-live-server-amd64.iso"
 * MEDIA_LABEL="Ubuntu Server 22.10 (Kinetic Kudu)"
 * MEDIA_SIZE="1642631168"
 */

#define _GNU_SOURCE
#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdnoreturn.h>
#include <sys/param.h>

#include <json-c/json.h>

int ubuntu_orange = COLOR_RED;
int text_white = COLOR_WHITE;
int back_green = COLOR_GREEN;

noreturn void usage(char *prog)
{
    fprintf(stderr, "usage: %s --input path --output path\n", prog);
    exit(1);
}

typedef struct _args_t
{
    char *infile;
    char *outfile;
} args_t;

args_t *args_create(int argc, char **argv)
{
    int cur = 1;
    args_t *args = calloc(sizeof(args_t), 1);

    while(cur < argc) {
        if(!strcmp(argv[cur], "--input") && cur + 1 < argc) {
            args->infile = strdup(argv[cur + 1]);
            cur += 2;
        } else if(!strcmp(argv[cur], "--output") && cur + 1 < argc) {
            args->outfile = strdup(argv[cur + 1]);
            cur += 2;
        } else {
            usage(argv[0]);
        }
    }
    if(!args->infile || !args->outfile) {
        usage(argv[0]);
    }

    return args;
}

void args_free(args_t *args)
{
    if(!args) return;

    free(args->infile);
    free(args->outfile);
    free(args);
}

void logger(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    FILE *out = fopen("/tmp/iso-chooser-menu.log", "a");
    vfprintf(out, fmt, ap);
    fprintf(out, "\n");
    fclose(out);
}

char *saprintf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *ret = NULL;
    vasprintf(&ret, fmt, ap);
    return ret;
}

typedef enum {
    DECREASE=-1,
    SELECT=0,
    INCREASE=1,
} choice_event;

typedef struct _iso_data_t
{
    char *label;
    char *url;
    int size;
} iso_data_t;

/* create the iso_data_t structure.  Caller allocates a free()able string, and
 * a later call to iso_data_free() will release both the iso_data_t and the
 * strings supplied here. */
iso_data_t *iso_data_create(char *label, char *url, int size)
{
    iso_data_t *ret = calloc(sizeof(iso_data_t), 1);
    if(!ret) return NULL;

    ret->label = label;
    ret->url = url;
    ret->size = size;
    return ret;
}

void iso_data_free(iso_data_t *iso_data)
{
    if(!iso_data) return;
    free(iso_data->label);
    free(iso_data->url);
    free(iso_data);
}

typedef struct _choices
{
    int len;
    int cur;
    iso_data_t **values;
} choices_t;

choices_t *choices_create(int len)
{
    choices_t *ret = (choices_t *)calloc(sizeof(choices_t), 1);
    if(!ret) return NULL;

    ret->values = (iso_data_t **)calloc(sizeof(iso_data_t *), len);
    if(!ret->values) {
        free(ret);
        return NULL;
    }
    ret->len = len;
    return ret;
}

void choices_free(choices_t *c)
{
    if(!c) return;

    for(int i = 0; i < c->len; i++) {
        iso_data_free(c->values[i]);
    }
    free(c);
}

int horizontal_center(int len)
{
    return (COLS - len) / 2;
}

int vertical_center(int len)
{
    /* accounts for 3 line banner */
    return 3 + (LINES - 3 - len) / 2;
}

void orange_banner(char *label)
{
    int x1 = 0;
    int w = COLS;

    /* Simulate the banner from Subiquity.
     * - draw black on orange for the half-block rows
     * - draw white on orange for the text row */
    short black_orange = 1;
    short white_orange = 2;
    init_pair(black_orange, COLOR_BLACK, ubuntu_orange);
    init_pair(white_orange, text_white, ubuntu_orange);

    cchar_t half_block_upper;
    setcchar(&half_block_upper, L"\u2580", 0, black_orange, NULL);

    cchar_t space;
    setcchar(&space, L" ", 0, white_orange, NULL);

    cchar_t half_block_lower;
    setcchar(&half_block_lower, L"\u2584", 0, black_orange, NULL);

    mvhline_set(0, x1, &half_block_upper, w);
    mvhline_set(1, x1, &space, w);
    mvhline_set(2, x1, &half_block_lower, w);

    attron(COLOR_PAIR(white_orange));
    mvaddstr(1, horizontal_center(strlen(label)), label);
    attroff(COLOR_PAIR(white_orange));
}

void button(int y, int x, char *label, int textwidth)
{
    char *button_text = saprintf("[ %-*s \u25b8 ]", textwidth, label);
    /* Simulate the appearance of buttons in Subiquity.  The unicode character
     * is the right-pointing smaller tringle arrow */
    mvaddstr(y, x, button_text);
    free(button_text);
}

void add_chooser(choices_t *choices, int selected)
{
    short white_green = 3;
    init_pair(white_green, text_white, back_green);

    int longest = 0;
    for(int i = 0; i < choices->len; i++) {
        longest = MAX(longest, (int)strlen(choices->values[i]->label));
    }
    /* The + 6 accounts for the button text around the label */
    int center_x = horizontal_center(longest + 6);
    int center_y = vertical_center(choices->len);
    for(int i = 0; i < choices->len; i++) {
        int y = center_y + i;
        if(i == selected) {
            attron(COLOR_PAIR(white_green));
        }
        button(y, center_x, choices->values[i]->label, longest);
        if(i == selected) {
            attroff(COLOR_PAIR(white_green));
        }
    }
}

int color_byte_to_ncurses(uint8_t color_byte)
{
    return color_byte / 255.0 * 1000;
}

void init_color_from_bytes(short color,
                           uint8_t byte_r, uint8_t byte_g, uint8_t byte_b)
{
    int nc_r = color_byte_to_ncurses(byte_r);
    int nc_g = color_byte_to_ncurses(byte_g);
    int nc_b = color_byte_to_ncurses(byte_b);
    init_color(color, nc_r, nc_g, nc_b);
}

void write_output(char *fname, iso_data_t *iso_data)
{
    FILE *f = fopen(fname, "w");
    if(!f) {
        logger("failed to open output file [%s]", fname);
        exit(1);
    }

    fprintf(f, "MEDIA_URL=\"%s\"\n", iso_data->url);
    fprintf(f, "MEDIA_LABEL=\"%s\"\n", iso_data->label);
    fprintf(f, "MEDIA_SIZE=\"%d\"\n", iso_data->size);
    fclose(f);
}

void choice_handle_event(args_t *args, choices_t *choices, choice_event evt)
{
    switch(evt) {
        case DECREASE:
            if(choices->cur > 0) {
                choices->cur--;
            }
            break;
        case SELECT:
            iso_data_t *cur = choices->values[choices->cur];
            write_output(args->outfile, cur);
            logger("selected:\n\t%s\n\t%s\n\t%d",
                    cur->label, cur->url, cur->size);
            break;
        case INCREASE:
            if(choices->cur < choices->len - 1) {
                choices->cur++;
            }
            break;
        default:
            logger("invalid event id [%d]", evt);
            exit(1);
    }
}

#define UNUSED(X) __attribute__((unused(X)))

char *find_largest_subkey(json_object *obj)
{
    char *ret = NULL;
    json_object_object_foreach(obj, key, val) {
        (void)val;
        if(!ret || strcmp(ret, key) < 0) {
            ret = key;
        }
    }
    return ret;
}

choices_t *read_iso_choices(char *filename)
{
    json_object *root = json_object_from_file(filename);
    if(!root) return NULL;

    json_object *products = json_object_object_get(root, "products");
    json_object *lunar = json_object_object_get(products,
            "com.ubuntu.cdimage.daily:ubuntu-server:daily-live:23.04:amd64");
    json_object *codename = json_object_object_get(lunar, "release_codename");
    json_object *title = json_object_object_get(lunar, "release_title");
    json_object *versions = json_object_object_get(lunar, "versions");
    char *recent = find_largest_subkey(versions);
    json_object *date = json_object_object_get(versions, recent);
    json_object *items = json_object_object_get(date, "items");
    json_object *iso = json_object_object_get(items, "iso");
    json_object *path = json_object_object_get(iso, "path");
    json_object *size = json_object_object_get(iso, "size");

    choices_t *choices = choices_create(2);
    choices->values[0] = iso_data_create(
            strdup("Ubuntu Server 22.10 (Kinetic Kudu)"),
            strdup("https://releases.ubuntu.com/kinetic/ubuntu-22.10-live-server-amd64.iso"),
            1642631168);
    choices->values[1] = iso_data_create(
            saprintf("Ubuntu Server %s (%s)",
                    json_object_get_string(title),
                    json_object_get_string(codename)),
            saprintf("http://cdimage.ubuntu.com/%s",
                    json_object_get_string(path)),
            json_object_get_int(size));

    json_object_put(root);
    return choices;
}

void exit_cb()
{
    erase();
    refresh();
    endwin();
}

int main(int argc, char **argv)
{
    args_t *args = args_create(argc, argv);
    setlocale(LC_ALL, "C.UTF-8");

    if(!initscr()) {
        logger("initscr failure");
        return 1;
    }

    atexit(exit_cb);

    noecho();

    if(!has_colors()) {
        logger("has_colors failure");
        endwin();
        return 1;
    }

    if(start_color() == ERR) {
        logger("start_color failure");
        endwin();
        return 1;
    }

    keypad(stdscr, TRUE);

    cbreak();

    curs_set(0); /* hide */
    refresh();

    logger("start");

    logger("can_change_color [%d]", can_change_color());
    if(can_change_color()) {
        init_color_from_bytes(ubuntu_orange, 0xE9, 0x54, 0x20);
        init_color_from_bytes(text_white, 0xFF, 0xFF, 0xFF);
        init_color_from_bytes(back_green, 0x0E, 0x84, 0x20);
    } else {
        /* These are terminal 256 color codes, see
         * https://www.ditig.com/256-colors-cheat-sheet for an example. */
        ubuntu_orange = 202;  /* not really but kinda close */
        text_white = 231;
        back_green = 28;
    }

    choices_t *iso_info = read_iso_choices(args->infile);

    bool continuing = true;
    int ch = 0;

    while(continuing) {
        orange_banner("Choose an Ubuntu version to install");
        add_chooser(iso_info, iso_info->cur);
        ch = getch();
        logger("got input [%d]\n", ch);
        switch(ch) {
            case KEY_DOWN:
                choice_handle_event(args, iso_info, INCREASE);
                break;
            case KEY_UP:
                choice_handle_event(args, iso_info, DECREASE);
                break;
            case '\f':  /* ctrl-L */
                redrawwin(stdscr);
                break;
            case KEY_ENTER:
            case '\r':
            case '\n':
            case ' ':
                choice_handle_event(args, iso_info, SELECT);
                continuing = false;
                break;
            default:
                break;
        }
    }

    choices_free(iso_info);
    args_free(args);

    return 0;
}
