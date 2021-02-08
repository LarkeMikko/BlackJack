/*
 * BlackJack.c
 *
 *  Created on: 10.10.2019
 *      Author: larke
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
// TODO: insert other definitions and declarations here
#define SUITS 4
#define VALUES 13
#define CARDS 52
#define UPLIMIT 52
#define LOWLIMIT 1
#define MAXHAND 8
#define PLAYERS 2
#define BLACKJACK 21
#define MINBET 5
#define MAXBET 100
#define ENDMARKER -1
#define DEALERMAX 17
#define YES 1
#define NO 2
#define STRLEN 256
#define ZERO 0

enum suits{spades,clubs,hearts,diamonds};
enum values{ace,two,three,four,five,six,seven,eight,nine,ten,jack,queen,king};
enum results{lose_bjack,win_bjack,draw_bjack,draw,win,lose};

typedef struct deck_{
	int suit;
	int value;
	bool drawn;
}deck;

void init_deck(deck *d);
int pick_random();
void deal_cards(int num,int *hand,deck *d);
int hand_value(int *hand);
void card_to_hand(int *hand, int card);
bool action(int *hand,deck *d);
void print_hand(int *hand, int value, bool is_dealer, deck *d);
int blackjack_win(bool p_black, bool d_black);
int win_cond(int player_hand,int dealer_hand);
int results(int win_cond,int bet,int credits);
const char* suit_name(int i);
const char* value_name(int i);
void reset_hand(int *hand);
bool read_int(int *num, int min, int max);
void removeNewline(char* string);

int main(void) {

#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif

    // TODO: insert code here
    deck d[CARDS];
    int win_condition;
    int credits=ZERO, bet=ZERO, quit=ZERO, rounds=LOWLIMIT;
    bool left_table=false;
    int player_hand[MAXHAND]={},dealer_hand[MAXHAND]={};
    int player_value=ZERO, dealer_value=ZERO;
    bool player_blackjack=false, dealer_blackjack=false;

    srand(time(0));

    printf("Welcome to my humble blackjack table.\n");

    do{
    	printf("how many credits you want to withdraw? (Minimum bet is %d credits and you can't withdraw more than %d credits.)\n",MINBET,MAXBET);
    }while(!read_int(&credits,MINBET,MAXBET));

    while(credits>=MINBET){

    	init_deck(&d);
    	reset_hand(player_hand);
    	reset_hand(dealer_hand);
    	player_value=ZERO;
    	dealer_value=ZERO;
    	player_blackjack=false;
    	dealer_blackjack=false;

    	printf("\n--ROUND %d BEGINS--\n",rounds);
    	rounds++;

    	do{
    	    printf("Name your bet. (Minimum bet is %d credits, you currently have %d credits)\n",MINBET,credits);
    	}while(!read_int(&bet,MINBET,credits));

    	deal_cards(2,player_hand,&d);
    	player_value=hand_value(player_hand);
    	if(player_value==BLACKJACK){
    		player_blackjack=true;
    	}
    	print_hand(player_hand,player_value,false,&d);

    	deal_cards(1,dealer_hand,&d);
    	dealer_value=hand_value(dealer_hand);
    	print_hand(dealer_hand,dealer_value,true,&d);

    	deal_cards(1,dealer_hand,&d);
    	dealer_value=hand_value(dealer_hand);
    	if(dealer_value==BLACKJACK){
    		dealer_blackjack=true;
    	}

    	win_condition=blackjack_win(player_blackjack,dealer_blackjack);

    	if(win_condition==draw){
    		while(!action(player_hand,&d)){
    			player_value=hand_value(player_hand);
    			print_hand(player_hand,player_value,false,&d);
    			if(player_value>BLACKJACK){
    				win_condition=lose;
    				break;
    			}else if(player_value==BLACKJACK){
    				win_condition=win;
    				break;
    			}
    		}
    		print_hand(dealer_hand,dealer_value,true,&d);
    		if(dealer_value<DEALERMAX){
    			printf("Dealers hand value is less than %d, they will draw until they reach that.\n",DEALERMAX);
    		}else{
    			printf("Dealers hand value have reached %d, they won't draw more cards.\n",DEALERMAX);
    		}

    		while(dealer_value<DEALERMAX){

    					deal_cards(1,dealer_hand,&d);
    					dealer_value=hand_value(dealer_hand);
    				}

    		win_condition=win_cond(player_value,dealer_value);

    	}

    	printf("\nRound ends with following hands:\n\n");
    	print_hand(player_hand,player_value,false,&d);
    	print_hand(dealer_hand,dealer_value,true,&d);

    	credits=results(win_condition,bet,credits);

    	if(credits>=MINBET){
    		do{
    			printf("Do you wish to continue? 1)yes 2)no\n");
    		}while(!read_int(&quit,YES,NO));
    		if(quit==NO){
    			left_table=true;
    			break;
    		}
    	}
    }
    if(left_table==true){
    	printf("You left the table with %d credits in your pocket.\n",credits);
    }else{
    	printf("You have only %d credits left, that is less than tables minimum bet.\n",credits);
    }

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
    }
    return 0 ;
}

void init_deck(deck *d){
	int suit_count=0, value_count=0, card_count=0;

	for(suit_count=0;suit_count<SUITS;suit_count++){
		for(value_count=0;value_count<VALUES;value_count++){

			d[card_count].suit=suit_count;
			d[card_count].value=value_count;
			d[card_count].drawn=false;
			card_count++;
		}
	}
}

int pick_random(){
	int r;
	r=(rand() % (UPLIMIT-LOWLIMIT+1))+LOWLIMIT;
	return r;
}


void deal_cards(int num,int *hand,deck *d){
	int i=0,r,value=0;
	while(i<num){
		r=pick_random();
		if(d[r].drawn==false){
			card_to_hand(hand,r);
			d[r].drawn=true;
			i++;
		}
	}
}


int hand_value(int *hand){
	int value=0,card,ace_count=0;

	for(int count=0;hand[count]!=ENDMARKER;count++){
		card=(hand[count]%VALUES)+1;
		if(card>=2 && card<=9){
			value+=card;
		}else if(card>=10 && card<=13){
			value+=10;
		}else{
			ace_count++;
			value+=11;
		}
	}
	while(value>BLACKJACK && ace_count>0){
		value-=10;
		ace_count--;
	}
	return value;
}

int blackjack_win(bool p_black, bool d_black){
	if(p_black==true && d_black==false){
		return win_bjack;
	}
	if(p_black==false && d_black==true){
		return lose_bjack;
	}
	if(p_black==true && d_black==true){
		return draw_bjack;
	}
	return draw;
}

int results(int win_cond,int bet,int credits){
	switch (win_cond){
	case lose_bjack:
		credits-=bet;
		printf("Dealer haves blackjack, you lose %d credits, current balance is %d credits.\n",bet,credits);
		return credits;
	case win_bjack:
		bet=(3*(bet*2))/2;
		credits+=bet;
		printf("You win with blackjack %d credits, current balance is %d credits.\n",bet,credits);
		return credits;
	case draw_bjack:
		printf("Blackjack draw, bets returned to you, current balance is %d credits.\n",credits);
		return credits;
	case draw:
		credits-=bet;
		printf("Draw, house wins, current balance is %d credits.\n",credits);
		return credits;
	case win:
		bet=bet*2;
		credits+=bet;
		printf("You win %d credits, current balance is %d credits.\n",bet,credits);
		return credits;
	case lose:
		credits-=bet;
		printf("You lose %d credits, current balance is %d credits.\n",bet,credits);
		return credits;
	}
}

const char* suit_name(int i){
	switch(i%SUITS){
	case spades:
		return "spades";
	case clubs:
		return "clubs";
	case hearts:
		return "hearts";
	case diamonds:
		return "diamonds";
	}
	return "none";
}

const char* value_name(int i){
	switch(i%VALUES){
	case ace:
		return "ace";
	case two:
			return "two";
	case three:
			return "three";
	case four:
			return "four";
	case five:
			return "five";
	case six:
			return "six";
	case seven:
			return "seven";
	case eight:
			return "eight";
	case nine:
			return "nine";
	case ten:
			return "ten";
	case jack:
		return "jack";
	case queen:
		return "queen";
	case king:
		return "king";
	}
	return "none";
}


void card_to_hand(int *hand, int card){
	int count;

	for(count=0;hand[count]!=ENDMARKER;count++);
	if(count<(MAXHAND-1)){
		hand[count+1]=hand[count];
		hand[count]=card;
	}
}

void print_hand(int *hand, int value, bool is_dealer, deck *d){
	if(is_dealer==false){
		printf("Player hand:\n");
	}else{
		printf("Dealer hand:\n");
	}
	for(int a=0;hand[a]!=ENDMARKER;a++){
		int r = hand[a];
		printf("%s of %s\n",value_name(d[r].value),suit_name(d[r].suit));
	}
	printf("hand value is %d\n\n",(value));
}

bool action(int *hand,deck *d){
	int num;
	do{
		printf("1)Hit 2)Stand\n");
	}while(!read_int(&num,YES,NO));

	switch(num){
	case YES:
		deal_cards(1,hand,d);
		break;
	case NO:
		return true;
	}
	return false;
}

void reset_hand(int *hand){
	hand[0]=ENDMARKER;
	for(int i=1;i<MAXHAND;i++){
		hand[i]=0;
	}
}

bool read_int(int *num, int min, int max){
	char str[STRLEN];
	fgets(str,STRLEN,stdin);
	removeNewline(str);
	int temp;

	if(sscanf(str, "%d", &temp)!=1 || temp<min || temp>max){
		printf("Your input need to be number between %d and %d.\n",min,max);
		return false;
	}
	*num=temp;
	return true;

}

int win_cond(int player_hand,int dealer_hand){
	if(player_hand>BLACKJACK){
		return lose;
	}else if(dealer_hand>BLACKJACK){
		return win;
	}else if(player_hand>dealer_hand){
		return win;
	}else if(dealer_hand>player_hand){
		return lose;
	}else if(player_hand==dealer_hand){
		return draw;
	}
}


void removeNewline(char* string){
	if(string[strlen(string)-1]=='\n'){
		string[strlen(string)-1]='\0';
	}
	if(string[strlen(string)-1]=='\r'){
		string[strlen(string)-1]='\0';
	}
}
