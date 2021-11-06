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

#include "args.h"
#include "system.h"
#include "error.h"
#include "config.h"
#include <assert.h>

void ARG_parse (int argc, char **argv, int n, const cfg_t **list) {
  int i, j;
  char *key;
  const cfg_t *c;
  assert(argc >= 0);
  assert(argv != NULL);
  assert(n >= 0);
  assert(list != NULL);
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] != 0) {
      j = 0;
      c = NULL;
      key = &argv[i][1];
      while (j < n && c == NULL){
        c = CFG_find_entry(key, list[j]);
        j++;
      }
      if (c == NULL) {
        ERR_failinit("%s: unknown parameter %s\n", argv[0], argv[i]);
      } else if (c->t == Y_SW_ON || c->t == Y_SW_OFF) {
        CFG_update_key(key, "on", c);
      } else if (i + 1 < argc) {
        CFG_update_key(key, argv[i + 1], c);
        i += 1;
      } else {
        ERR_failinit("%s: missing argument for parameter %s\n", argv[0], argv[i]);
      }
    } else {
      ERR_failinit("%s: something wrong here: %s\n", argv[0], argv[i]);
    }
  }
}
