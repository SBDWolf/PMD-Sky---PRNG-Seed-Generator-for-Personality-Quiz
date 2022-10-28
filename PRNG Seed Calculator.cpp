//Author: SBDWolf. Get angry at him for using global variables and making the code confusing.
#include <iostream>
#include <conio.h>
#include <cstring>
using namespace std;

/*
This program simulates the algorithm that generates questions for the personality quiz in PMD Sky, and displays the seeds that generate the fastest questions.
At the moment, this program and research applies to the European version only.

First I need to talk about the global PRNG. It's a simple 16-bit value that is stored at RAM address $020AF7CC.
The save data keeps the value of a PRNG seed (at address $30). If a save file is present, that PRNG seed will get loaded at that RAM address as-is shortly after getting past the title screen.
Keep in mind that even after deleting the save data in-game, the save file itself will still be kept, along with a PRNG value.
The only situation where a save file is not present is if the game has never ever been saved yet, or if you hack the save file out, or if you're playing on emulator and you manually remove the save file.
If a save file is not present, then the game will simply keep advancing the PRNG value that had been worked on during the title screen sequence.
The PRNG algorithm is a very simple Linear Congruential Generator algorithm, with parameters x(n+1)=(109*x(n)+1021)%65536 (where x(n+1) is the value of the new x and x(n) is the current value of x).
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

*/

//List of questions
string qtable[64]={"You're running a marathon, and at the start you fall flat on your face! What will you do?","You're at a movie theater. What are you there to see?","What's your studying style?",
"You've been asked to do a difficult task. What will you do?","Are you truly sincere when you apologize?","Do you think you have good study habits?",
"There's a rumor around about a ghost haunting the school bathrooms! What do you do?","Everyone's sharing a dessert, and there's an extra piece. What do you do?",
"How are your mornings?","Everyone around you is laughing hard at something you think is pretty boring. What do you do?",
"You want to reveal that you like someone a whole bunch! What do you do?","You're on a stroll when a TV crew pounces on you for an interview. What do you do?",
"Do you like lively parties?","The people at the next table are singing for someone's birthday. What do you do?",
"You're in a completely silent assembly when you suddenly hear someone pass gas! How do you react?","Do you like karaoke?",
"Do you prefer to play outside rather than inside?","You see a big and comfortable bed. Your first reaction is to...",
"You're hiking up a mountain when you reach diverging paths. Which kind do you take?","You discover a beat-up-looking treasure chest in some ruins. What do you do?",
"Your friend takes a spectacular fall! What do you do?","You're daydreaming...when your friend sprays you with water! What do you do?",
"Have you ever thought that if you dug in your backyard you could find buried treasure?","Have you ever told a joke that just completely fell flat?",
"Hey, what's that? There's someone behind you! So...did you look just now?","Your friend has made a meal that tastes terrible. They ask, 'How is it?' You say...?",
"You run into a new person that you haven't talked to very much before. What do you do?","Good news and bad news... Which one do you want to hear first?",
"Have you ever realized you were hogging the conversation?","When you see a switch, do you feel an overwhelming urge to flip it?",
"You find something at a great bargain price! What do you do?","You're packing your classroom's snacks for a picnic when you get hungry. What do you do?",
"Do you want to be taller someday?","Do you think it's important to always aim to be the best?","Have you ever looked at your reflection in a mirror and thought, 'What a cool person'?",
"If you saw someone doing something bad, could you scold them?","Do you prefer to be busy or to have a lot of free time?",
"You're about to buy a popular game when someone else gets the last copy! How do you feel?",
"You notice that a toy you bought for full price yesterday is marked down to half price today! How do you feel?","Your friend is running a little late to meet you. Is that OK?",
"You've just stuffed yourself with a good meal when a great dessert arrives. What do you do?","You have a really important test tomorrow! What do you do?",
"You've been invited to a wonderful party. It's time for the party to start, but there's nobody there! You think...?","Do you think that, no matter what, life goes on?",
"The phone's ringing! What do you do?","Your friends seem to be making plans to hang out, just out of earshot. You think...","Do you want to be famous?",
"You've been handed a large bag as a souvenir. What do you do?","Have you ever forgotten you bought something and bought another one?",
"Once you've decided something, do you see it through to the end?","Have you ever said 'nice to meet you' to someone you've met previously?","Did you make any New Year's resolutions?",
"Do you think it's important to be fashionably late?","You've won big on a raffle ticket! You say...","Do you think blaming something you did on someone else is sometimes necessary?",
"Your friend suddenly won't listen to you, when everything was fine yesterday. What happened?","You take off your shoes to realize your socks are two different colors! What do you do?",
"You fail miserably! Everyone found out, and they're disappointed in you... What do you do?","Have you ever accidentally called a teacher 'Mom' or 'Dad'?",
"Have you ever blurted something out without thinking about the consequences first?","You're in class when you realize that you really have to go to the restroom! What do you do?",
"Your friend offers to treat you to dinner. What do you do?","You see a cake that is past its expiration date, but only by one day. What do you do?",
"You're eating at a very fancy restaurant known for its food. Which course do you select?"};


//Yes global variables, I'm a bad coder. It's bad practice, I know.
//Array that contains the seeds previously generated by the program, for the purpose of finding the "next fastest" seed
int gseeds[65535];
//Array that contains the question "groups"(?) that have already been generated. If a flag in this array has already been set that corresponds to a value generated at a step of the genqs function,
//it will make that function's algorithm repeat a few steps.
//70 is an arbitrary excessive length because I don't know how many "groups"(?) there exist.
bool qsflags[70];
//Array that temporarily contains the questions generated by each seed.
int questions[8];
//Array that contains the questions of the fastest seed found
int fastestQuestions[8];
//PRNG seed
int seed;

//Function that advances the PRNG
void advancePRNG(){
    seed=(109*seed+1021)%65536;
}

//Function that generates questions.
int genqs(){
    int group;
    int temp;
    bool validity=true;
    //See above for the purpose of this array. Gets entirely initialized to false now.
    for(int i=0;i<70;i++){
        qsflags[i]=false;
    }
    //Generate 8 questions
    for(int i=0;i<8;i++){
        do{
            validity=true;
            advancePRNG();
            temp=seed*16;
            temp=temp>>16;
            //If a question of the same "group"(?) has already been generated, repeat this while loop
            if(qsflags[temp]==true){
                validity=false;
            }
        }while(validity==false);
    group=temp;
    qsflags[group]=true;
    advancePRNG();
    temp=seed*4;
    temp=temp>>16;
    //This resulting value is one of the 8 questions generated
    questions[i]=temp+(group<<2);
    }

}

//This function calculates the total string length of each question, and saves it as the fastest one if it's the shortest one yet
void calcSpeed(int r, int *minCharCount, int *fastestSeed, int gseedsCount){

    int totalChars=0;
    for (int i=0;i<8;i++){
        totalChars+=qtable[questions[i]].length();
    }

    //This block of code excludes seeds that have been previously generated and displayed on the program.
    bool seedValidity=true;
    for (int y=0;y<gseedsCount;y++){
        if(gseeds[y]==r){
            seedValidity=false;
        }
    }


    //saves a seed as the fastest one if it produces the shortest total string length from questions
    if(totalChars<*minCharCount&&seedValidity==true){
        *minCharCount=totalChars;
        *fastestSeed=r;
        for (int q=0;q<8;q++){
            fastestQuestions[q]=questions[q];
        }
    }

}

int readInt(){
	int input;
	bool inputValidity;
	do {
		cin>>input;
		inputValidity=true;
		if(cin.fail()){
			cout<<"Invalid Character."<<endl;
			cin.clear();
			cin.ignore(1000, '\n');
			inputValidity=false;
		}
		else{
			cin.ignore(1000, '\n');
			if(cin.gcount()>1){
				cout<<"Invalid Number Type."<<endl;
				inputValidity=false;
			}
		if ((input<0)&&inputValidity==true){
			cout<<"Number is negative."<<endl;
			inputValidity=false;
		}

		}

	}while(inputValidity==false);
	return input;
}



int main(){
    int gseedsCount=0;
    cout<<"Insert number of PRNG advances.\n(MelonDS 0.9.3, DeSmuME 0.9.11, DeSmuME 0.9.11 with dynamic recompiler set at 100, 3DS and OG DS are around 1378, 1231, 1167, 1360 and 1359 respectively. Toy with this value a little if it doesn't work): ";
    int PRNGAdvances=readInt();

    while(true){
        //Fastest seed found, initialized to a random variable
        int fastestSeed=100000;
        //Character count of the fastest seed, initialized to a random variable
        int minCharCount=2000;
        //This for loop brute forces each of the 65536 possible PRNG seeds



    for(int r=0;r<65536;r++){
        seed=r;
    	//This for loop refers to the amount of times the rng is advanced between the title screen and the moment the questions get generated.
    	//Seems like DS advances the rng fewer times than MelonDS 0.9.3. MelonDS gets around 1378, a real DS is around 1360. This amount is a little inconsistent, so to find a DS seed...
    	//...I had to toy with this value a bit. Generate a seed, try it out, if it doesn't work change this value a little. :/
    	//Desmume 0.9.11 is around 1231
        for (int i=0;i<PRNGAdvances;i++){
            advancePRNG();
        }
        genqs();
        calcSpeed(r, &minCharCount, &fastestSeed, gseedsCount);
    }

    //Outputs information on screen about the fastest seed found
    cout<<"Fastest seed:"<<fastestSeed<<"("<<minCharCount<<" characters)"<<endl;
    cout<<"Questions:"<<endl;
    for (int e=0;e<8;e++){
            //Just formats the output so that it adds an extra space if the question number is single digit, to make each string start from the same column
            if(fastestQuestions[e]<10){
                cout<<fastestQuestions[e]<<":  "<<qtable[fastestQuestions[e]]<<endl;
            }
            else {
                cout<<fastestQuestions[e]<<": "<<qtable[fastestQuestions[e]]<<endl;
            }

    }
    //Stores the generated seed into an array, so that it doesn't get taken into account on the next execution
    gseedsCount++;
    gseeds[gseedsCount-1]=fastestSeed;

    cout<<"Press any key to generate a new seed.\n\n";
	getch();
    }
	return 0;
}

/*
qtable[0]=  "You're running a marathon, and at the start you fall flat on your face! What will you do?";
qtable[1]=  "You're at a movie theater. What are you there to see?";
qtable[2]=  "What's your studying style?"
qtable[3]=  "You've been asked to do a difficult task. What will you do?"
qtable[4]=  "Are you truly sincere when you apologize?"
qtable[5]=  "Do you think you have good study habits?"
qtable[6]=  "There's a rumor around about a ghost haunting the school bathrooms! What do you do?"
qtable[7]=  "Everyone's sharing a dessert, and there's an extra piece. What do you do?"
qtable[8]=  "How are your mornings?"
qtable[9]=  "Everyone around you is laughing hard at something you think is pretty boring. What do you do?"
qtable[10]= "You want to reveal that you like someone a whole bunch! What do you do?"
qtable[11]= "You're on a stroll when a TV crew pounces on you for an interview. What do you do?"
qtable[12]= "Do you like lively parties?"
qtable[13]= "The people at the next table are singing for someone's birthday. What do you do?"
qtable[14]= "You're in a completely silent assembly when you suddenly hear someone pass gas! How do you react?"
qtable[15]= "Do you like karaoke?"
qtable[16]= "Do you prefer to play outside rather than inside?"
qtable[17]= "You see a big and comfortable bed. Your first reaction is to..."
qtable[18]= "You're hiking up a mountain when you reach diverging paths. Which kind do you take?"
qtable[19]= "You discover a beat-up-looking treasure chest in some ruins. What do you do?"
qtable[20]= "Your friend takes a spectacular fall! What do you do?"
qtable[21]= "You're daydreaming...when your friend sprays you with water! What do you do?"
qtable[22]= "Have you ever thought that if you dug in your backyard you could find buried treasure?"
qtable[23]= "Have you ever told a joke that just completely fell flat?"
qtable[24]= "Hey, what's that? There's someone behind you! So...did you look just now?"
qtable[25]= "Your friend has made a meal that tastes terrible. They ask, 'How is it?' You say...?"
qtable[26]= "You run into a new person that you haven't talked to very much before. What do you do?"
qtable[27]= "Good news and bad news... Which one do you want to hear first?"
qtable[28]= "Have you ever realized you were hogging the conversation?"
qtable[29]= "When you see a switch, do you feel an overwhelming urge to flip it?"
qtable[30]= "You find something at a great bargain price! What do you do?"
qtable[31]= "You're packing your classroom's snacks for a picnic when you get hungry. What do you do?"
qtable[32]= "Do you want to be taller someday?"
qtable[33]= "Do you think it's important to always aim to be the best?"
qtable[34]= "Have you ever looked at your reflection in a mirror and thought, 'What a cool person'?"
qtable[35]= "If you saw someone doing something bad, could you scold them?"
qtable[36]= "Do you prefer to be busy or to have a lot of free time?"
qtable[37]= "You're about to buy a popular game when someone else gets the last copy! How do you feel?"
qtable[38]= "You notice that a toy you bought for full price yesterday is marked down to half price today! How do you feel?"
qtable[39]= "Your friend is running a little late to meet you. Is that OK?"
qtable[40]= "You've just stuffed yourself with a good meal when a great dessert arrives. What do you do?"
qtable[41]= "You have a really important test tomorrow! What do you do?"
qtable[42]= "You've been invited to a wonderful party. It's time for the party to start, but there's nobody there! You think...?"
qtable[43]= "Do you think that, no matter what, life goes on?"
qtable[44]= "The phone's ringing! What do you do?"
qtable[45]= "Your friends seem to be making plans to hang out, just out of earshot. You think..."
qtable[46]= "Do you want to be famous?"
qtable[47]= "You've been handed a large bag as a souvenir. What do you do?"
qtable[48]= "Have you ever forgotten you bought something and bought another one?"
qtable[49]= "Once you've decided something, do you see it through to the end?"
qtable[50]= "Have you ever said 'nice to meet you' to someone you've met previously?"
qtable[51]= "Did you make any New Year's resolutions?"
qtable[52]= "Do you think it's important to be fashionably late?"
qtable[53]= "You've won big on a raffle ticket! You say..."
qtable[54]= "Do you think blaming something you did on someone else is sometimes necessary?"
qtable[55]= "Your friend suddenly won't listen to you, when everything was fine yesterday. What happened?"
qtable[56]= "You take off your shoes to realize your socks are two different colors! What do you do?"
qtable[57]= "You fail miserably! Everyone found out, and they're disappointed in you... What do you do?"
qtable[58]= "Have you ever accidentally called a teacher 'Mom' or 'Dad'?"
qtable[59]= "Have you ever blurted something out without thinking about the consequences first?"
qtable[60]= "You're in class when you realize that you really have to go to the restroom! What do you do?"
qtable[61]= "Your friend offers to treat you to dinner. What do you do?"
qtable[62]= "You see a cake that is past its expiration date, but only by one day. What do you do?"
qtable[63]= "You're eating at a very fancy restaurant known for its food. Which course do you select?"

qtable[0]= 89;
qtable[1]= 53;
qtable[2]= 27;
qtable[3]= 59;
qtable[4]= 41;
qtable[5]= 40;
qtable[6]= 83;
qtable[7]= 73;
qtable[8]= 22;
qtable[9]= 93;
qtable[10]= 71;
qtable[11]= 82;
qtable[12]= 27;
qtable[13]= 80;
qtable[14]= 97;
qtable[15]= 20;
qtable[16]= 49;
qtable[17]= 63;
qtable[18]= 83;
qtable[19]= 76;
qtable[20]= 53;
qtable[21]= 76;
qtable[22]= 86;
qtable[23]= 57;
qtable[24]= 73;
qtable[25]= 84;
qtable[26]= 86;
qtable[27]= 62;
qtable[28]= 57;
qtable[29]= 67;
qtable[30]= 60;
qtable[31]= 88;
qtable[32]= 33;
qtable[33]= 57;
qtable[34]= 86;
qtable[35]= 61;
qtable[36]= 55;
qtable[37]= 89;
qtable[38]= 110;
qtable[39]= 61;
qtable[40]= 91;
qtable[41]= 58;
qtable[42]= 115;
qtable[43]= 48;
qtable[44]= 36;
qtable[45]= 83;
qtable[46]= 25;
qtable[47]= 61;
qtable[48]= 68;
qtable[49]= 64;
qtable[50]= 71;
qtable[51]= 40;
qtable[52]= 51;
qtable[53]= 45;
qtable[54]= 78;
qtable[55]= 92;
qtable[56]= 87;
qtable[57]= 90;
qtable[58]= 59;
qtable[59]= 82;
qtable[60]= 92;
qtable[61]= 58;
qtable[62]= 85;
qtable[63]= 88;*/

