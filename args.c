/*
 * Copyright 2022-2023 Canonical Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "args.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <syslog.h>

#include <sys/stat.h>

bool file_exists(const char *path)
{
    struct stat statbuf = {};
    if(stat(path, &statbuf) == -1) {
        fprintf(stderr, "expected file %s not found\n", path);
        return false;
    }
    return true;
}

void args_free(args_t *args)
{
    if(!args) return;
    free(args->infiles);
    free(args);
}

args_t *args_create(int argc, char **argv)
{
    args_t *args = calloc(sizeof(args_t), 1);
    if(!args) {
        syslog(LOG_ERR, "fatal: alloc failure");
        exit(1);
    }

    int cur = 1;
    args->outfile = argv[cur++];

    args->num_infiles = argc - cur;
    if(args->num_infiles < 1) {
        args_free(args);
        return NULL;
    }

    args->infiles = calloc(sizeof(char *), args->num_infiles);
    if(!args->infiles) {
        syslog(LOG_ERR, "fatal: alloc failure");
        exit(1);
    }

    for(int i = 0; i < args->num_infiles; i++) {
        args->infiles[i] = argv[cur++];
        if(!file_exists(args->infiles[i])) {
            args_free(args);
            return NULL;
        }
    }

    return args;
}
