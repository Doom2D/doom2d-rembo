#!/bin/sh
#
#  Copyright (C) Andriy Shinkarchuck <adriano32.gnu@gmail.com> 2011
#
#  This file is part of the Doom2D:Rembo project.
#
#  Doom2D:Rembo is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2 as
#  published by the Free Software Foundation.
#
#  Doom2D:Rembo is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, see <http://www.gnu.org/licenses/> or
#  write to the Free Software Foundation, Inc.,
#  51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
#
set -e
WORK_DIR=`pwd`
if [ $# -eq 0 ]; then
  DEST_DIR=/usr/share/doom2d-rembo
  if [ -d $DEST_DIR ]; then
    if [ "$(ls -A $DEST_DIR)" ]; then
      echo "Error: "$DEST_DIR" is not empty. Please, check it content and delete if it is unnecesary before starting"
      exit 1
    fi
  else
    mkdir $DEST_DIR
    chmod 755 $DEST_DIR
  fi
  if [ -d $DEST_DIR ]; then
    if [ -d $WORK_DIR/../music ]; then
      cp -R $WORK_DIR/../music $DEST_DIR/music
      cp $WORK_DIR/doom2d.wad $DEST_DIR
      cp $WORK_DIR/megadm.wad $DEST_DIR
      cp $WORK_DIR/superdm.wad $DEST_DIR
      cp $WORK_DIR/default.cfg $DEST_DIR;
      echo "Copying data files to "$DEST_DIR" completed succesfully"
    else
      echo "music subdirectory was not found in "$WORK_DIR". Check you source tarball consistency";
    fi
  else
    echo $DEST_DIR" does not exist or yet not been created. Please, check your rights and create it befory executing.";
  fi
fi
