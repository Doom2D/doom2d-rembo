/* Copyright (C) 2020 SovietPony
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License ONLY.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include "glob.h"
#include "system.h"

const cfg_t *CFG_find_entry (const char *key, const cfg_t *cfg);
int CFG_update_key (const char *key, const char *value, const cfg_t *cfg);

int CFG_open_iterator (const char *name);
int CFG_scan_iterator (char *key, int keylen, char *value, int valuelen);
void CFG_close_iterator (void);

int CFG_read_config (const char *name, int n, const cfg_t **cfg);
int CFG_update_config (const char *old, const char *new, int n, const cfg_t **cfg, const char *msg);

#endif /* CONFIG_H_INCLUDED */
