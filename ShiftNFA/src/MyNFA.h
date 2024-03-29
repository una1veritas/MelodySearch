//============================================================================
// Name        : MyNFA.cpp
// Author      : Ayane
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#ifndef SRC_MYNFA_H_
#define SRC_MYNFA_H_

// ↓入力"*++--+-++==---#+++b==+-=b+-+--+-+-+-+b+==-+-==b==" "++--+-++==---#+++b==+-=b+-+--+-"
//最後に[-]を追加

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

#include "bset64.h"

using std::cout;
using std::endl;
using std::string;

struct MyNFA {
private:
	/* 整数のビット長により設定する上限 */
	static constexpr unsigned int STATE_LIMIT = 64;
	static constexpr unsigned int ALPHABET_LIMIT 	= 128;  /* =,+,-,#,b */
	#define bit64(x)  (uint64_t(1)<<(x))
	#define bit64(x)  (uint64_t(1)<<(x))

	/* 定数 */
	static constexpr unsigned int TRANSITION_NOT_DEFINED =	0;
	static constexpr unsigned int STATE_IS_NOT_FINAL = 	0;
	static constexpr unsigned int STATE_IS_FINAL 	= 1;

	static constexpr char alphabet[] = "#+-=b";
	/* 状態は 数字，英大文字を含む空白 (0x20) から _ (0x5f) までの一文字 */
	/* に対応させる正の整数 {0,...,63} の要素に限定. */
	/* 文字は ASCII 文字, char 型の {0,...,127} の要素に限定. */
	bset64 delta[STATE_LIMIT][ALPHABET_LIMIT];	/* 遷移関数 : Q x Σ -> 2^Q*/
	bset64  initials; 								/* 初期状態 */
	bset64 finals;			 					/* 最終状態 */
	unsigned int size;
	bset64 current;                           /* 現在の状態の集合　*/
	bool suffix_dontcare;

public:
	/* パターン文字列から nfa を初期化 */
	MyNFA(const string & melody) {
		string pattern = "";
		size = 0;
		for(const char & c : melody) {
			if ( c != '*' ) {
				pattern += c;
				++size;
			} else {
				if ( pattern.length() == 0 or pattern.back() != '*' ) {
					pattern += '*';
				}
			}
		}
		//std::cout << endl << "statesize = " << statesize << endl;

		/* データ構造の初期化 */
		for(unsigned int i = 0; i < STATE_LIMIT; ++i) {
			for(unsigned int a = 0; a < ALPHABET_LIMIT; ++a) { /* = 1 からでもよいが */
				delta[i][a] = 0; 	/* 空集合に初期化 */
			}
		}
		initials.set(0);
		finals.set(size);

		unsigned int pos = 0;
		for(unsigned int i = 0; i < pattern.length(); i++){
			char c = pattern[i];

			if(c == '+'){
				delta[pos][(int)'+'] |= bit64(pos+1);
			}else if(c == '#'){
				delta[pos][(int)'#'] |= bit64(pos+1);
			}else if(c == '^'){
				delta[pos][(int)'+'] |= bit64(pos+1);
				delta[pos][(int)'#'] |= bit64(pos+1);
			}else if(c == '-'){
				delta[pos][(int)'-'] |= bit64(pos+1);
			}else if(c == 'b'){
				delta[pos][(int)'b'] |= bit64(pos+1);
			}else if(c == '_'){
				delta[pos][(int)'-'] |= bit64(pos+1);
				delta[pos][(int)'b'] |= bit64(pos+1);
			}else if(c == '='){
				delta[pos][(int)'='] |= bit64(pos+1);
			} else if ( c == '*' ) {
				// VLDC '*'
				for(const char * p = alphabet; *p != 0 ; ++p) {
					delta[pos][(int) *p] |= bit64(pos);
				}
			}
			if ( c != '*') {
				// the state transition for '*' is just the self loop,
				// otherwise it advances the state to the next
				++pos;
			}
		}
		suffix_dontcare = (( pattern.back() == '*' ) ? true : false);
	}

	friend std::ostream & operator<<(std::ostream & out, const MyNFA & m) {
		bset64 states(0);
		bool alphabet[MyNFA::ALPHABET_LIMIT];

		for(unsigned int a = 0; a < MyNFA::ALPHABET_LIMIT; ++a) {
			alphabet[a] = false;
		}
		for(unsigned int i = 0; i < MyNFA::STATE_LIMIT; ++i) {
			for(unsigned int a = 0; a < MyNFA::ALPHABET_LIMIT; ++a) {
				if ( m.delta[i][a] != 0 ) {
					states |= bit64(i);
					states |= m.delta[i][a];
					alphabet[a] = true;
				}
			}
		}
		out << "nfa(" << "states = " << states.str() << endl;
		out << "alphabet = {";
		int count = 0;
		for(unsigned int i = 0; i < ALPHABET_LIMIT; ++i) {
			if ( alphabet[i] == true) {
				if (count)
					printf(", ");
				out << char(i);
				++count;
			}
		}
		out << "}," << endl;

		out << "delta = " << endl;
		out << "state symbol| next" << endl;
		out << "------------+------" << endl;
		for(unsigned int i = 0; i < MyNFA::STATE_LIMIT; ++i) {
			for(unsigned int a = 0; a < MyNFA::ALPHABET_LIMIT; ++a) {
				if ( m.delta[i][a] != 0 ) {
					out << " " << std::setw(3) << i << " ,  " << char(a) << "   | " << m.delta[i][a].str() << endl;
				}
			}
		}
		out << "------------+------" << endl;
		out << "initial state = " << m.initials.str() << endl;
		out << "final states = " << m.finals.str() << endl;
		out << ")" << endl;
		return out;
	}

	void reset() {
		current = initials;
	}

	bset64 transfer(char a) {
		bset64 next = 0;
//		for(int i = 0; i < STATE_LIMIT; ++i) {
//			if ((current & (1<<i)) != 0) {
//				if (delta[i][(int)a] != 0) /* defined */
//					next |= delta[i][(int)a];
//				//else /* if omitted, go to and self-loop in the ghost state. */
//			}
//		}
		bset64 state = current;

		while(state != 0){
			unsigned int i = state.ctz();
//			count++;

			if (delta[i][(int)a] != 0){
				next |= delta[i][(int)a];
			}
			state.clsb();
		}
		return current = next;
	}

	int accepting() {
		return (finals & current) != 0;
	}

	long int run(const string & inputstr) {
		long pos = 0;

		reset();
		//std::cout << current.str() ;
		for (auto itr = inputstr.begin(); itr != inputstr.end(); ++itr) {
			transfer(*itr);
			//std::cout << "-" << char(*ptr) << "["<< pos << "]" << "-> "<< current.str() ;
			if ( suffix_dontcare and accepting() ) break;
			pos++;
		}
		//std::cout << endl;

		if (accepting()) {
			return pos;
		} else {
			return -1;
		}
	}

};

#endif
