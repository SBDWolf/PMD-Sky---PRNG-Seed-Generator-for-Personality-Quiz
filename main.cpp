//Author: SBDWolf
#include <iostream>
#include <fstream>
#include <conio.h>
using namespace std;

/*This program simply simulates the PRNG algorithm and prints the result of each advancement to a file.
I used this program in conjuction with Bizhawk 2.8's memory watch on the PRNG seed to understand how many times the seed would get advanced before the questions go generated.
I'd look at the PRNG value the frame before the questions get generated, ctrl+f for the seed in the file, and add 2 to that number of advancement (since the frame the questions get generated,
the seed gets advanced two more times.

*/

int main(){
    //This commented out seed is the value that the game initializes the seed to when it first boots up. If no save file is present, this seed will be used to calculate the questions.
	//int x=13452;
	ofstream myfile ("output.txt");
  if (myfile.is_open())
  {
      //change this value to change the seed being used.
      int x=13047;
	myfile<<"Starting seed"<<": "<<x<<endl;
	for (int i=0;i<5000;i++){
        x=(109*x+1021)%65536;
        if(i%2==0){
            myfile<<"Advancement "<<i+1<<": "<<x<<endl;
        }
        else{
            x=(109*x+1021)%65536;
            myfile<<"-Advancement "<<i+1<<": "<<x<<endl;
        }

	}
    myfile.close();
    cout<<"Successfully wrote to file."<<endl;
  }
  else {
        cout << "Unable to open file";
        return 1;
        }
	cout<<"Press any key to end...";
	getch();
	return 0;
}

