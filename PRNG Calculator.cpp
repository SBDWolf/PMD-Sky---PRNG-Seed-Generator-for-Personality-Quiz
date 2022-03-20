//Author: Daniele Budano
#include <iostream>
#include <conio.h>
using namespace std;

int main(){

	//int x=13452;
	int x=39803;
	cout<<0<<": "<<x<<endl;
	for (int i=0;i<1500;i++){
		x=(109*x+1021)%65536;
//		if (x==29339){
			cout<<"Intermediate: "<<x<<endl;
//		}
		x=(109*x+1021)%65536;
//		if (x==29339){
		
		cout<<i+1<<": "<<x<<endl;
//	}
	}
	cout<<"Press any key to end...";
	getch();
	return 0;
}

