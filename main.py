import pygame
import os
import time
import subprocess
import sys

from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler


global keys_file
keys_file = None

global on
on = 0

class MyHandler(FileSystemEventHandler):
    def __init__(self, sound_files_map, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.sound_files_map = sound_files_map

    def on_modified(self, event):
        global keys_file
        global on
        if keys_file is None:
            keys_file = open(event.src_path, "r");

        key = keys_file.readline().strip()

        if key in self.sound_files_map:
            if key == "V":
                soundObj = pygame.mixer.Sound(self.sound_files_map[key][on])
                on = (on + 1) % 2
            else:
                soundObj = pygame.mixer.Sound(self.sound_files_map[key])
            soundObj.play()
        #print(f'event type: {event.event_type}  path : {event.src_path}')


cur_dir = os.path.dirname(os.path.abspath(__file__))

exec_file = "/build/bin/lmeca2710_project"

sound_dir = "sound"

# Choose the theme you prefer

theme = "synthwave.wav"
#theme = "chill.wav"

# Sound effects

sfm = dict(A=os.path.join(cur_dir, sound_dir, "pew.wav"),
           D=os.path.join(cur_dir, sound_dir, "boum.wav"),
           V=(os.path.join(cur_dir, sound_dir, "turn_on.wav"),
              os.path.join(cur_dir, sound_dir, "turn_off.wav"))
)

pygame.mixer.init()


if __name__ == "__main__":

    event_handler = MyHandler(sfm)
    observer = Observer()
    observer.schedule(event_handler, path=os.path.join(cur_dir, "data"), recursive=False)
    observer.start()

    args = ["." + os.path.join(cur_dir, exec_file)]
    args.extend(sys.argv[1:])
    process = subprocess.Popen(args)

    file = os.path.join(cur_dir, sound_dir, theme)
    soundObj = pygame.mixer.Sound(file)

    time.sleep(1)

    soundObj.play(-1)
    while process.poll() is None:
        time.sleep(0.5)
    soundObj.stop()

    if keys_file is not None:
        keys_file.close()
