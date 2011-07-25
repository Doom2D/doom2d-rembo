#!/bin/sh
WORK_DIR=`pwd`
if (($#==0)); then
  DEST_DIR=/usr/share/doom2d-rembo ;
  mkdir /usr/share/doom2d-rembo ;
  if [ -d $DEST_DIR ]; then
    if [ -d $WORK_DIR/../music ]; then
      cp -R $WORK_DIR/../music $DEST_DIR/music
      cp $WORK_DIR/doom2d.wad $DEST_DIR
      cp $WORK_DIR/megadm.wad $DEST_DIR
      cp $WORK_DIR/superdm.wad $DEST_DIR
      cp $WORK_DIR/default.cfg $DEST_DIR;
    else
      echo "music subdirectory was not found in "$WORK_DIR". Check you source tarball consistency";
    fi
  else
    echo $DEST_DIR" does not exist or yet not been created. Please, check your rights and create it befory executing.";
  fi
fi
