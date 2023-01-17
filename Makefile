CC = arm-linux-gnueabihf-gcc
FLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -D_BSD_SOURCE -D_DEFAULT_SOURCE
EXECUTEABLE = intercom
LCD_EXECUTABLE = lcd
PUBDIR = $(HOME)/cmpt433/public/myApps
OUTDIR = $(PUBDIR)
PTHREAD = -pthread
LDFLAGS=-L${HOME}/cmpt433/SQLite_for_ARM/lib -lpthread -lm

SQLITE = -lsqlite3
MYAPPS = $(HOME)/cmpt433/public/myApps

AUDIOSRC = src/audio
DATABASESRC = src/database
KEYPADSRC = src/keypad
VIDEOSRC = src/video



SRC = src/main.c src/hardware/uart.c src/linux/bbg_linux.c src/cellular/sim7600.c src/cellular/phone.c src/linux/bbg_time.c src/hardware/lock.c src/hardware/keypad.c src/video/display.c src/database/database.c src/linux/bbg_gpio.c src/hardware/mode.c src/hardware/joystick.c src/hardware/directory.c src/linux/bbg_a2d.c

DATABASE_PATH = src/database/intercom.db

EXECUTEABLE = intercom

all: lcd intercom

lcd:
	make --directory=src/video/liquidCrystalCrossCompile


all: intercom

intercom: ${SRC}
	${CC} ${FLAGS} ${SRC} -o ${EXECUTEABLE} ${LDFLAGS} ${SQLITE}
	cp ${EXECUTEABLE} $(MYAPPS)/

watchdog_run: ${SRC}
	${CC} ${FLAGS} watchdog/watchdog.c -o watchdog_run ${LDFLAGS} 
	cp watchdog_run $(MYAPPS)/

install: 
	sudo apt-get install libsqlite3-dev

clean:
	rm ${EXECUTEABLE}
	rm -f $(OUTDIR)/$(LCD_EXECUTABLE)
	rm $(MYAPPS)/${EXECUTEABLE}
	rm watchdog_run
	rm $(MYAPPS)/watchdog_run
