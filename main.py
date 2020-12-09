import pygame
import os
import time
import subprocess
import sys
import random

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
        if keys_file is None and event.src_path.endswith(".keys.txt"):
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

sfm_default = dict(A=os.path.join(cur_dir, sound_dir, "pew.wav"),
           D=os.path.join(cur_dir, sound_dir, "boum.wav"),
           V=(os.path.join(cur_dir, sound_dir, "turn_on.wav"),
              os.path.join(cur_dir, sound_dir, "turn_off.wav"))
)

sfm_youpidou = dict(A=os.path.join(cur_dir, sound_dir, "piou_piou.wav"),
           D=os.path.join(cur_dir, sound_dir, "on_aime_pas_bis.wav"),
           V=(os.path.join(cur_dir, sound_dir, "ohoui.wav"),
              os.path.join(cur_dir, sound_dir, "il_est_mort.wav")),
           QUIT=os.path.join(cur_dir, sound_dir, "on_va_manger.wav"),
           ENTER=os.path.join(cur_dir, sound_dir, "it_must_work.wav")
)

random_talks = [
    os.path.join(cur_dir, sound_dir, "aucune_chance.wav"),
    os.path.join(cur_dir, sound_dir, "complique.wav"),
    os.path.join(cur_dir, sound_dir, "piece_of_cake_inc.wav"),
    os.path.join(cur_dir, sound_dir, "puree.wav"),
    os.path.join(cur_dir, sound_dir, "vilain_vilain.wav"),
    os.path.join(cur_dir, sound_dir, "youpidou.wav"),
    os.path.join(cur_dir, sound_dir, "youtubepointcomhein.wav"),
    os.path.join(cur_dir, sound_dir, "voila_voila.wav"),
    os.path.join(cur_dir, sound_dir, "ca_va_prendre_du_temps.wav"),
    os.path.join(cur_dir, sound_dir, "aiou.wav"),
    os.path.join(cur_dir, sound_dir, "biiiiiiiyoup.wav"),
]

pygame.mixer.init()


if __name__ == "__main__":

    if "-y" in sys.argv:
        mode = "YOUPIDOU"
        sfm = sfm_youpidou
    else:
        mode = "default"
        sfm = sfm_default

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

    if mode == "YOUPIDOU":
        soundObj_enter = pygame.mixer.Sound(sfm["ENTER"])
        channel = soundObj_enter.play()

        while channel.get_busy():
            time.sleep(0.1)
        soundObj_enter.stop()
        soundObj.set_volume(0.5)

    soundObj.play(-1)

    soundObj_random = None
    channel_random = None
    last_sound = None

    while process.poll() is None:
        time.sleep(0.5)

        if mode == "YOUPIDOU":
            if channel_random is None or not channel_random.get_busy():
                if channel_random:
                    soundObj_random.stop()
                will_play = random.random() > 0.8
                if will_play:
                    r_sound = random.choice(random_talks)
                    while r_sound == last_sound:
                        r_sound = random.choice(random_talks)
                    last_sound = r_sound
                    soundObj_random = pygame.mixer.Sound(r_sound)
                    channel_random = soundObj_random.play()
    soundObj.stop()
    if soundObj_random:
        soundObj_random.stop()

    if keys_file is not None:
        keys_file.close()

    if mode == "YOUPIDOU":
        soundObj = pygame.mixer.Sound(sfm["QUIT"])
        channel = soundObj.play(fade_ms=300)

        while channel.get_busy():
            time.sleep(0.1)
        soundObj.stop()
