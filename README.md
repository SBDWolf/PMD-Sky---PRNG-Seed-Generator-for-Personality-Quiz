# PMD-Sky---PRNG-Seed-Generator-for-Personality-Quiz
A C++ program that helps in finding optimal PRNG Seeds for the personality quiz in PMD Sky. Compile and run in your C++ IDE of choice (I use Codeblocks).

Taken directly from the comments in the program itself:

This program simulates the algorithm that generates questions for the personality quiz in PMD Sky, and displays the seeds that generate the fastest questions.
At the moment, this program and research applies to the European version only.

First I need to talk about the global PRNG. It's a simple 16-bit value that is stored at RAM address $020AF7CC.
The save data keeps the value of a PRNG seed (at address $30). If a save file is present, that PRNG seed will get loaded at that RAM address as-is shortly after getting past the title screen.
Keep in mind that even after deleting the save data in-game, the save file itself will still be kept, along with a PRNG value.
The only situation where a save file is not present is if the game has never ever been saved yet, or if you hack the save file out, or if you're playing on emulator and you manually remove the save file.
If a save file is not present, then the game will simply keep advancing the PRNG value that had been worked on during the title screen sequence.
The PRNG algorithm is a very simple Linear Congruential Generator algorithm, with parameters x(n+1)=(109\*x(n)+1021)%65536 (where x(n+1) is the value of the new x and x(n) is the current value of x).
Every frame that is spent idle during the intro, the PRNG is typically advanced once per frame. If you are holding ANY button on the controller, it is typically advanced twice per frame.

With this in mind, the speedrunning strat to manipulate the quiz is to hold B at all times during the intro and buffer the "New game" screen (20-frame window) to essentially aim to advance the
PRNG a consistent, predictable amount of time before the questions get generated.
This amount of times varies depending on platforms, and even on the same platforms there may be weird small differences between different seeds that I haven't quite been able to understand.
The frame the questions get generated, the following happens:
0. The PRNG is advanced two times (one time if you weren't holding B). In this program I integrated this step into the first 'for' loop (see later);
1. The PRNG is advanced one time;
2. A register takes its value from the PRNG seed. This register is multiplied it by 16, then an Arithmetic Right Shift is performed on the resulting value by 16 bits.
3. It seems like there's a system in place where certain questions can't be generated if this resulting value has already been generated before. The game checks for this, and goes back to
step 1 if so. Is this a way to prevent questions from the same "group" from getting generated? This is purely a guess from me, I didn't spend much time thinking about this besides
observing the algorithm of it;
5. The PRNG is advanced an additional time;
6. The result of step 2 is multiplied by the value of the PRNG seed, and an Arithmetic Right Shift is performed on the resulting value by 16 bits;
7. This value is then added to the 2-bit-Logical-Left-Shifted value resulted from step 2. The result of this step is the finalized question value. At the bottom of this program there's a commented
out table that shows more clearly the corresponding index value to each question.
8. The algorithm restarts from step 1, until 8 questions have been generated.

I found most of this information by using Bizhawk 2.8's memory watch, debugger and trace log features. Trace logging on the frame that questions got generated was particularly crucial.
The PRNG algorithm function is apparently located at $02002250 in ROM, while the part of code that handles the question generation is located at about $0238B360 (all according to the trace log).

This program assumes that you were playing the intro like in a speedrun: holding B at all times, and buffering the "New game" screen. With these parameters, it then brute forces each of the 65536
possible PRNG seeds, and spits out the fastest one out of all based on character length of all the questions. No check is done on natures that can be obtained from the questions, or how many times
you have to select responses different from the top one. I've found factors manually after generating the seeds.

The way I've used this program is, I used a different C++ program and memory watch on Bizhawk 2.8 with the MelonDS 0.9.3 core to understand how many times PRNG was advanced between the moment it
got loaded into a save file and the moment the questions were generated. I then generated a seed, tried it, and it worked fine.
I then tried that seed on a 2DS, but it didn't work because a real DS advances the PRNG a different amount of times than emulator. I used that same program to understand by how many times that was
(around 19), then adjusted this program with that in mind, generated some new seeds, and tried them. I was still missing the predicted questions, and that's when I found out that there's some
weirdness that can cause different seeds to advance the PRNG by slightly different amounts for some reason. I just manually brute forced slightly different PRNG advances, until I found that
a PRNG seed that gets advanced 1360 times worked perfectly on DS in this case.

To actually try a seed out, I first converted the seed into hexadecimal, then put it into the save file at address $30 in little endian (that is, right-most byte first, then left-most byte second)
with a hex editor (I use HxD).
I then fixed the checksum by opening the save file in Sky Editor (make sure to select "Let me choose file type" or it won't recognize the save file) and then saving the file. Sky Editor automatically
fixes the checksum as soon as you open a save file with it. If you don't do this, the game will see the save file as corrupted, and it will either restore a copy of the save data located at address
$C800 in the save file, or if it sees that part as corrupted as well it will delete your save data.
