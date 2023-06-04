import os
from fastapi import FastAPI

app = FastAPI()

# Variable to store the current song
current_song = None
playing = False

@app.get('/play')
def play_song():
    global current_song
    global playing

    if playing:
        return {'message': 'playing'}

    if current_song is None:
        return {'message': 'No song selected'}

    # Play the selected song using os.system
    os.system(f"nohup play {current_song} &")
    playing = True

    return {'message': f'Playing {current_song}'}

@app.get('/stop')
def stop_song():
    global playing
    if playing:
        # Stop the currently playing song using os.system
        os.system("pkill play")
        playing = False
        return {'message': 'Song stopped'}

    return {'message': 'No song playing'}

@app.get('/choose/{song}')
def choose_song(song: str):
    global current_song
    global playing
    if playing:
        return {'message': 'A song is already playing'}

    # Check if the song exists in the songs directory
    song_path = f"/music/{song}"
    if not os.path.exists(song_path):
        return {'message': f'Song "{song}" not found'}

    current_song = song_path
    return {'message': f'Selected {song}'}

