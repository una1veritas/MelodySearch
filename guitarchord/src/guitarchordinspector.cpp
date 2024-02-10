#define _GLIBCXX_DEBUG
//#include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;
//using std::cout;

constexpr static int SF[6][23] = {
	{29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51},//1弦
	{24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46},//2弦
	{20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42},//3弦
	{15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37},//4弦
	{10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32},//5弦
	{5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27}//6弦
};

struct fingers { //指の構造体
	vector<pair<int, int>> press; //押さえるフレット(first)と弦番号(second)のペア
	bool push; //指が弦を押さえているかどうか

	fingers() : press(), push(false) {}

	void clear() {
		press.clear();
		push = false;
	}

	void push_back(pair<int, int> fb) {
		press.push_back(fb);
		push = true;
	}
};

struct form { //コードフォームの構造体
	fingers fing[5]; //指の構造体の配列（コードフォーム）
	int w = 0; //難易度
};

struct d_fing { //隣り合う指の幅の構造体
	pair<int, int> nei_f;
	int d = 0;
};

//コードの音が与えられ、そこから指板上の押さえるフレットと弦のペア（フレット、弦）のvectorを返す
vector<vector<pair<int, int>>> candchord(vector<int>& chord) { //	指板上の押さえる位置を見つける
	vector<pair<int, int>> fb; //指板
	vector<vector<pair<int, int>>> list_fb; //指板の候補
	int j = 0;
	typedef pair<int, int> pair;
	int min = 14;
	int max = 0;
	/*if (chord.size() > 6) {
		printf("ギター一本では弾けない");
		return list_fb;
	}*/
	sort(chord.begin(), chord.end()); //音を昇順に並べ替え
	for (int rs = 5; rs >= 3; rs--) { //ルート音をどの弦に設定するか
		int i = 0;
		for (int s = rs; s >= 0; s--) { //ルート音から決める。
			for (int fret = 0; fret <= 11; fret++) {
				if (chord[i] % 12 == SF[s][fret] % 12) { //i番目の音と指板上の位置の対応付け
					if (fret == 0) {
						fb.push_back(pair(fret, s));
						i++;
						break;
					}
					else if (fret < min && fret > max) {
						min = fret;
						max = fret;
						fb.push_back(pair(fret, s));
						i++;
						break;
					} else if (fret <= min && max - fret <= 3) { //minは割り当て済みの指板上の最小の位置
						min = fret;
						fb.push_back(pair(fret, s));
						i++;
						break;
					}
					else if (fret >= max && fret - min <= 3) { //maxは割り当て済み指板上の最大の位置
						max = fret;
						fb.push_back(pair(fret, s));
						i++;
						break;
					}
					else if (fret <= max && fret >= min){
						fb.push_back(pair(fret,s));
						i++;
						break;
					}
					else {
						break;
					}
				}
			}
			if (fb.size() == chord.size() || fb.size() == rs + 1) {
				list_fb.push_back(fb);
				break;
			}
		}
		min = 14;
		max = 0;
		/*for (int k = 0; k < fb.size(); k++) {
			cout << "(" << fb[k].first << "フレット, " << fb[k].second + 1 << "弦)";
		}*/
		fb.erase(fb.begin(), fb.end());
		//cout << endl;
	}
	/*for (int l = 0; l < list_fb.size(); l++) {
		for (int k = 0; k < list_fb[l].size(); k++) {
			cout << "(" << list_fb[l][k].first << "フレット, " << list_fb[l][k].second + 1 << "弦)";
		}
		cout << endl;
	}*/
	
	return list_fb;
}

//不自然な指板情報(開放弦があるとき、押さえるフレットの弦の最小値が4よりも大きい指板)かを判定
bool natural(vector<pair<int, int>> fb) {
	int min = 99;
	bool flag = true;

	if (fb.size() < 6 && fb.size() > 3 && fb[0].second == 5) {
		return false;
	}
	for (int i = 0; i < fb.size(); i++) {
		if (fb[i].first != 0 && fb[i].first < min) {
			min = fb[i].first;
		}
		else if (fb[i].first == 0 && min != 99 && min > 4) {
			flag = false;
		}
		else {
			continue;
		}
	}
	return flag;
}

//指板情報が与えられ、開放弦の弦番号をまとめたvectorを返す
vector<int> open(vector<pair<int, int>> fb) { //開放弦の弦番号を管理する
	vector<int> s;
	for (int a = 0; a < fb.size(); a++) { //firstにフレット,secondに弦番号を想定
		if (fb[a].first == 0) {
			s.push_back(fb[a].second);
		}
	}
	return s;
}

//指板情報が与えられ、そこからセーハ可能区間のペア（セーハ可能な最初の弦番号,セーハ可能な最後の弦番号）を返す。
vector<pair<int, int>> barre(vector<pair<int, int>> fb) {
	vector<pair<int, int>> newfb(fb), seha, tfb(fb);
	int min, i, start, end;
	vector<int> ava_seha, exprored_f;
	for (int a = 0; a < fb.size(); a++) {						//
		newfb[a] = make_pair(-fb[a].second, fb[a].first);		//
	}															//  pairのfirstに弦番号、secondにフレットとする
	sort(newfb.begin(), newfb.end());							//　弦番号の降順にソート
	for (int b = 0; b < newfb.size(); b++) {					//　
		tfb[b] = make_pair(-newfb[b].first, newfb[b].second);	//
	}															//
	
	for (start = 0; start < tfb.size() - 1; start++) { //セーハできる箇所を探す
		i = start;
		min = tfb[start].second;
		while (i < tfb.size()) {
			if (tfb[i].second >= min && tfb[i].second != 0) {
				i++;
			}
			else {
				break;
			}
		}
		end = i - 1;
		if (end - start >= 2) {
			seha.push_back(pair<int, int>(tfb[start].first, tfb[end].first));
			ava_seha.push_back(tfb[start].second);
		}
	}
	return seha;
}

//ある弦とセーハ可能区間が与えられ、ある弦がセーハ可能区間のvectorの先頭の要素としてあればtrueをそれ以外はfalseを返す。
bool find(vector<pair<int, int>> seha, int string) {
	for (int i = 0; i < seha.size(); i++) {
		if (seha[i].first == string) {
			return true;
		}
	}
	return false;
}

//押さえている指の本数を返す
int num(fingers fing[5]) {
	int k = 0;
	for (int i = 0; i < 5; i++) {
		if (fing[i].push == true) {
			k++;
		}
	}
	return k;
}

//与えられたコードフォームから押さえているフレットの幅を返す
int width_all(fingers fing[5]) {
	int width, min = 100, max = 0;
	for (int i = 0; i < 5; i++) {
		if (fing[i].push == true) {
			if (fing[i].press[0].first < min && fing[i].press[0].first != 0) {
				min = fing[i].press[0].first;
			}
			else if (fing[i].press[0].first > max) {
				max = fing[i].press[0].first;
			}
			else {
				continue;
			}
		}
	}
	width = max - min + 1;
	return width;
}

//与えられたコードフォームからセーハしている指の番号を返す
vector<int> f_seha(fingers fing[5]) {
	vector<int> f;
	for (int i = 0; i < 5; i++) {
		if (fing[i].push == true) {
			if (fing[i].press.size() > 1) {
				f.push_back(i);
			}
		}
	}
	return f;
}

//与えられたコードフォームから隣り合う指の弦番号が離れているかを調べ、離れている指のペアを返す。
vector<pair<int, int>> leave(fingers fing[5]) {
	vector<pair<int, int>> neighbor;
	int k = 0;
	for (int i = 0; i < 4; i++) {
		if (fing[i].push == true && fing[i + 1].push == true) {
			if (fing[i].press.size() == 1 && fing[i].press.size() == 1) {
				if (fing[i].press[0].second - fing[i + 1].press[0].second > abs(1)) {
					neighbor.push_back(make_pair(i, i + 1));
				}
			}
		}
	}
	return neighbor;
}

//与えられたコードフォームから、弦を押さえている指の中で隣り合う指のフレットの幅が広い指のペアを返す。
vector<d_fing> width_next(fingers fing[5]) {
	d_fing dfing;
	vector<d_fing> vec_fing;
	int k;
	for (int i = 0; i < 5; i++) {
		k = i + 1;
		if (fing[i].push == true) {
			for ( ; i + k < 5; k++) {
				if (fing[i + k].push == true) {
					if (abs(fing[i].press[0].first - fing[i + k].press[0].first) > k) {
						dfing.nei_f = make_pair(i, i + k);
						dfing.d = abs(fing[i].press[0].first - fing[i + k].press[0].first);
						vec_fing.push_back(dfing);
					}
				}
			}
		}
	}
	return vec_fing;
}

//与えた指板情報から、押さえ方として、各指に押さえる弦とフレットの組を与える。
fingers press_fingers(vector<pair<int, int>> fb, fingers fing[5]) {
	vector<pair<int, int>> newfb(fb), tfb(fb); //並べ替え後の指板
	vector<int> open_s = open(fb);
	vector<pair<int, int>> ava_seha = barre(fb);
	pair<int, int> p;
	unsigned int i, d;
	int j = 0;

	for (int a = 0; a < fb.size(); a++) {						//
		newfb[a] = make_pair(fb[a].first, -fb[a].second);		//
	}															//  フレットの昇順、フレットの値が同じ値の時は
	sort(newfb.begin(), newfb.end());							//　弦番号の降順にソート
	for (int b = 0; b < newfb.size(); b++) {					//　
		tfb[b] = make_pair(newfb[b].first, -newfb[b].second);	//
	}															//

	/*for (int k = 0; k < tfb.size(); k++) {
		cout << "(" << tfb[k].first << "フレット, " << tfb[k].second + 1 << "弦)";
	}
	cout << endl;*/

	if (tfb.size() - open_s.size() <= 4) { //押さえる音が4つ以下　セーハを極力使わない
		for (i = 0; i < tfb.size(); i++) { //セーハを使わない(指の幅は1以下)
			//printf("(1)\n");
			if (j > 3) {
				break;
			}
			if (tfb[i].first == 0) { //i番目の弦が開放弦
				continue;
			}
			else if(j == 0) {
				fing[j].push_back(tfb[i]);
				j++;
			}
			else if (tfb[i].first - tfb[i - 1].first == 0) {
				fing[j].push_back(tfb[i]);
				j++;
			}
			else {
				d = tfb[i].first - tfb[i - 1].first;
				j += d - 1;
				if (j < 4) {
					fing[j].push_back(tfb[i]);
					j++;
				}
				else {
					i = 10;
				}
			}
		}
		if (i == tfb.size()) {
			return *fing;
		} //(1)はここまで
		else { //(2)セーハを使う
			//printf("(2)\n");
			for (int k = 0; k < 5; k++) { //
				fing[k].clear();          //指の押さえる組をクリア
			}                             //
			j = 0;
			for (i = 0; i < tfb.size(); i++) {
				if (j > 3) {
					break;
				}
				if (tfb[i].first == 0) { //i番目の弦が開放弦
					continue;
				}
				else if (j == 0) { //人差し指
					bool it = find(ava_seha, tfb[i].second);
					if (it == true) { //セーハができるかを判断
						for (int k = 0; k <= ava_seha.size(); k++) {
							if (tfb[i].second == ava_seha[k].first) {
								p = ava_seha[k];
								break;
							}
						}
						int a = i;
						for (int k = p.first; k >= p.second; k--) {

							pair<int, int> np = make_pair(tfb[a].first, k);
							
							fing[0].push_back(np);
							if (tfb[i].first == np.first && tfb[i].second == np.second) {
								i++;
							}
						}
						j++;
						i--;
					}
					else { //セーハできない場合
						fing[j].push_back(tfb[i]);
						j++;
					}
				}
				else if (tfb[i].first - tfb[i - 1].first == 0) {
					fing[j].push_back(tfb[i]);
					j++;
				}
				else if (tfb[i].first - tfb[i - 1].first < 3) {
					d = tfb[i].first - tfb[i - 1].first;
					j += d - 1;
					if (j == 2) { //薬指でセーハを用いる
						bool it = find(ava_seha, tfb[i].second);
						if (it == true) { //セーハができるかを判断
							for (unsigned int k = 0; k <= ava_seha.size(); k++) {
								if (tfb[i].second == ava_seha[k].first) {
									p = ava_seha[k];
									break;
								}
							}
							int a = i;
							for (int k = p.first; k >= p.second; k--) {
								pair<int, int> np = make_pair(tfb[a].first, k);
								fing[j].push_back(np);
								if (tfb[i].first == np.first && tfb[i].second == np.second) {
									i++;
								}
							}
							j++;
							i--;
						}
						else {
							fing[j].push_back(tfb[i]);
							j++;
						}
					}
					else{
						fing[j].push_back(tfb[i]);
						j++;
					}
				}
				else {
					d = tfb[i].first - tfb[i - 1].first;
					j += d - 1;
					fing[j].push_back(tfb[i]);
					j++;
				}
			}
		} if (i == tfb.size()) {
			return *fing;
		} //(2)はここまで
		else {
			for (int k = 0; k < 5; k++) { //
				fing[k].clear();          //指の押さえる組をクリア
			}                             //
			j = 0;
			//printf("(3)\n");
			for (i = 0; i < tfb.size(); i++) { //(3)セーハを使わない(指の幅は2以下)
				if (j > 3) {
					break;
				}
				if (tfb[i].first == 0) { //i番目の弦が開放弦
					continue;
				}
				else if (j == 0) {
					fing[j].push_back(tfb[i]);
					j++;
				}
				else if (tfb[i].first - tfb[i - 1].first < 3) {
					fing[j].push_back(tfb[i]);
					j++;
				}
				else {
					d = tfb[i].first - tfb[i - 1].first;
					j += d - 1;
					fing[j].push_back(tfb[i]);
					j++;
				}
			}
			if (i == tfb.size()) {
				return *fing;
			}
		}
	}
	else { //押さえる音が5つ以上　セーハを用いる
		//printf("(4)\n");
		for (i = 0; i < tfb.size(); i++) {
			if (j > 3) {
				break;
			}
			if (tfb[i].first == 0) { //i番目の弦が開放弦
				continue;
			}
			else if (j == 0) { //人差し指
				bool it = find(ava_seha, tfb[i].second);
				if (it == true) { //セーハができるかを判断
					for (unsigned int k = 0; k <= ava_seha.size(); k++) {
						if (tfb[i].second == ava_seha[k].first) {
							p = ava_seha[k];
							break;
						}
					}
					int a = i;
					for (int k = p.first; k >= p.second; k--) {
						pair<int, int> np = make_pair(tfb[a].first, k);
						fing[0].push_back(np);
						if (tfb[i].first == np.first && tfb[i].second == np.second) {
							i++;
						}
					}
					j++;
					i--;
				}
				else { //セーハできない場合
					fing[j].push_back(tfb[i]);
					j++;
				}
			}
			else if (tfb[i].first - tfb[i - 1].first == 0) {
				fing[j].push_back(tfb[i]);
				j++;
			}
			else if (tfb[i].first - tfb[i - 1].first < 3) {
				d = tfb[i].first - tfb[i - 1].first;
				j += d - 1;
				if (j == 2) { //薬指でセーハを用いる
					bool it = find(ava_seha, tfb[i].second);
					if (it == true) { //セーハができるかを判断
						for (unsigned int k = 0; k <= ava_seha.size(); k++) {
							if (tfb[i].second == ava_seha[k].first) {
								p = ava_seha[k];
								break;
							}
						}
						int a = i;
						for (int k = p.first; k >= p.second; k--) {
							pair<int, int> np = make_pair(tfb[a].first, k);
							//cout << "(" << np.first << "フレット," << np.second + 1 << "弦)" << endl;
							fing[j].push_back(np);
							if (tfb[i].first == np.first && tfb[i].second == np.second) {
								i++;
							}
						}
						j++;
						i--;
					}
					else {
						fing[j].push_back(tfb[i]);
						j++;
					}
				}
				else {
					fing[j].push_back(tfb[i]);
					j++;
				}
			}
			else {
				d = tfb[i].first - tfb[i - 1].first;
				j += d - 1;
				fing[j].push_back(tfb[i]);
				j++;
			}
		} if (i == tfb.size()) {
			return *fing;
		}
		return *fing;
	}
	//
	return *fing;
}

//与えられたコードフォームからそのコードフォームの難しさとそのコードフォーム自身を構造体にまとめて返す
form eval_single(fingers fing[5]) {
	form c_form;
	int w = 0, w_n = 0, w_seha = 0, w_leave = 0, w_wida = 0, w_widn = 0;
	vector<int> seha;
	vector<pair<int, int>> l_pair;
	vector<d_fing> dis_fing;


	/*for (int k = 0; k < 5; k++) {
		if (fing[k].push != false) {
			for (int j = 0; j < fing[k].press.size(); j++) {
				cout << k << "番目の指が押さえる組 (" << fing[k].press[j].first << "フレット," << fing[k].press[j].second + 1 << "弦)" << endl;
			}
			cout << endl;
		}
	}*/

	//num関数から難易度推定
	if (num(fing) > 3) {
		w_n = 2;
	}
	else if (num(fing) > 2) {
		w_n = 1;
	}
	else {
		w_n = 0;
	}

	//f_seha関数から難易度推定
	seha = f_seha(fing);
	if (seha.size() > 1) {
		w_seha = 10;
	}
	else if (seha.size() == 1) {
		if (seha[0] == 0) {
			w_seha = 5;
		}
		else {
			w_seha = 8;
		}
	}

	//leave関数から難易度推定
	l_pair = leave(fing);
	if (l_pair.size() == 3) {
		w_leave = 6;
	}
	else if (l_pair.size() == 2) {
		w_leave = 4;
	}
	else if (l_pair.size() == 1) {
		if (l_pair[0] == make_pair(2, 3)) {
			w_leave = 4;
		}
		else {
			w_leave = 3;
		}
	}

	//width_all関数から難易度推定
	if (width_all(fing) == 4) {
		if (seha.size() != 0) {
			w_wida = 7;
		}
		else {
			w_wida = 5;
		}
	}
	else if (width_all(fing) == 3) {
		w_wida = 2;
	}

	//width_next関数から難易度推定
	dis_fing = width_next(fing);
	if (dis_fing.size() > 1) {
		w_widn = 20;
	}
	else if (dis_fing.size() == 1) {
		if (dis_fing[0].nei_f == make_pair(0, 1)) {
			w_widn = 10;
		}
		else {
			w_widn = 15;
		}
	}

	//ここまでの推定した各難易度からコードの難易度を推定
	w = max({ w_widn, w_wida, 0 }) + max({ w_n, w_seha, w_leave, 0 });

	for (int i = 0; i < 5; i++) {
		c_form.fing[i] = fing[i];
	}
	c_form.w = w;

	return c_form;
}

//与えられた二つのコード情報から、二つのコードのコードチェンジの難易度を返す。
double eval_multiple(form before, vector<int> a_cc, form &after) {
	double w;
	int d = 0, w_a = 0, w_seha = 0, up = 0, down = 0, stay = 0, change = 0, up_down = 0, n_efect = 0, w_state = 0, n = 0;
	after.w = 999;

	vector<vector<pair<int, int>>> list_afb; //指板
	list_afb = candchord(a_cc); //CC後の指板状態の決定

	fingers a_fing[5]; //コードフォーム
	form a_form; //仮のコードフォームとその難易度

	for (unsigned int k = 0; k < list_afb.size(); k++) {
		for (int l = 0; l < 5; l++) { //
			a_fing[l].clear();        //指の押さえる組をクリア
		}                             //

		press_fingers(list_afb[k], a_fing);

		a_form = eval_single(a_fing);
		if (a_form.w < after.w && natural(list_afb[k]) != false) {
			after.w = a_form.w;
			for (int i = 0; i < 5; i++) {
				after.fing[i] = a_form.fing[i];
			}
		}
	}
	//cout << "CC後のコードの難易度" << after.w << "ポイント" << endl;
	
	//最小フレットの変化から難易度推定
	if (after.fing[0].press[0].first != before.fing[0].press[0].first) {
		d = abs(after.fing[0].press[0].first - before.fing[0].press[0].first);
		if (d > 6) {
			w_a = 5;
		}
		else if(d > 3) {
			w_a = 4;
		}
		else if (d > 2) {
			w_a = 3;
		}
		else if (d > 1) {
			w_a = 2;
		}
	}

	//各指の状態変化から難易度推定
	for (int i = 0; i < 5; i++) {
		if (after.fing[i].push == true) {
			n++;
		}
	}
	for (int i = 0; i < 5; i++) {
		if (after.fing[i].push == true && before.fing[i].push == true) { //CC前とCC後でどちらも指iが弦を押さえているとき
			if (after.fing[i].press.size() > 1 && before.fing[i].press.size() > 1) { //どちらもセーハを行っている場合
				stay++;
			}
			else if (after.fing[i].press.size() > 1 && before.fing[i].press.size() > 1) { //どちらか片方がセーハを行っている場合
				w_seha = 3;
			}
			else {
				if (after.fing[i].press[0].second - before.fing[i].press[0].second > 0) { //弦番号が大きくなった
					up++;
				}
				else if (after.fing[i].press[0].second - before.fing[i].press[0].second < 0) { //弦番号が小さくなった
					down++;
				}
				else if(after.fing[i].press[0].second == before.fing[i].press[0].second && after.fing[i].press[0].first == before.fing[i].press[0].first ) { //変化がないとき
					stay++;
				}
			}
		}
		else if(after.fing[i].push == true && before.fing[i].push == false) { //CC後に指iが弦を押さえるとき
			change++;
		}
		else if(after.fing[i].push == false && before.fing[i].push == true) { //CC後に指iが弦を押さえなくなるとき
			change++;
		}
		else { //CC前CC後共に弦を押さえていないとき
			n_efect++;
		}
	}
	if (up == 0 || down == 0) {
		up_down = 0;
	}
	else {
		up_down = abs(up - down);
	}
	if (up_down + change > 2) {
		w_state = 3;
	}
	else if(up_down + change > 1) {
		w_state = 2;
	}
	else {
		w_state = 1;
	}

	if (up == 0 && down == 0 && change == 0) { //CC前後でコードが変わらないとき、
		w = 0;
	}
	else {
		if (n != 0) {
			w = double(after.w) * (1 - (double(stay) / double(n))) + w_seha + w_state;
		}
		else {
			w = after.w + w_seha + w_state;
		}
		
	}
	return w;
}

//与えられたコード情報のリストから、そのリストの最大難易度を返す。
double eval_part(vector<vector<int>> chord_list) {
	vector<vector<pair<int, int>>> list_fb; //指板
	form before, after;
	before.w = 999;
	fingers b_fing[5];
	fingers a_fing[5]; //コードフォーム
	form b_form; //仮のコードフォームとその難易度
	double w, max = 0;
	
	for (unsigned int i = 0; i < chord_list.size(); i++) {
		if (i == 0) { //コードリストの最初のコードの処理
			list_fb = candchord(chord_list[i]);
			for (unsigned int k = 0; k < list_fb.size(); k++) {
				for (int l = 0; l < 5; l++) { //
					b_fing[l].clear();        //指の押さえる組をクリア
				}                             //

				press_fingers(list_fb[k], b_fing);
				b_form = eval_single(b_fing);
				if (b_form.w < before.w && natural(list_fb[k]) != false) {
					before.w = b_form.w;
					for (int l = 0; l < 5; l++) {
						before.fing[l] = b_form.fing[l];
					}
				}
			}
			/*for (int k = 0; k < 5; k++) {
				if (before.fing[k].push != false) {
					for (int j = 0; j < before.fing[k].press.size(); j++) {
						cout << k << "番目の指が押さえる組 (" << before.fing[k].press[j].first << "フレット," << before.fing[k].press[j].second + 1 << "弦)" << endl;
					}
					cout << endl;
				}
			}
			cout << "CC前のコードの難易度" << before.w << "ポイント" << endl;*/
		}
		else {
			/*for (int k = 0; k < 5; k++) {
				if (before.fing[k].push != false) {
					for (int j = 0; j < before.fing[k].press.size(); j++) {
						cout << "before" << endl;
						cout << k << "番目の指が押さえる組 (" << before.fing[k].press[j].first << "フレット," << before.fing[k].press[j].second + 1 << "弦)" << endl;
					}
					cout << endl;
				}
			}
			cout << "CC前のコードの難易度" << before.w << "ポイント" << endl;*/
			
			w = eval_multiple(before, chord_list[i], after);
			/*for (int k = 0; k < 5; k++) {
				if (after.fing[k].push != false) {
					for (int j = 0; j < after.fing[k].press.size(); j++) {
						cout << "after" << endl;
						cout << k << "番目の指が押さえる組 (" << after.fing[k].press[j].first << "フレット," << after.fing[k].press[j].second + 1 << "弦)" << endl;
					}
					cout << endl;
				}
			}*/
			//cout << "コード進行の難易度" << w << "ポイント" << endl;
			if (max < w) {
				max = w;
			}
			before.w = after.w;
			for (int l = 0; l < 5; l++) { 
				before.fing[l] = after.fing[l];
			}

			after.w = 0;
			for (int l = 0; l < 5; l++) { //
				after.fing[l].clear();    //指の押さえる組をクリア
			}                             //
		}
	}
	return max;
}

double eval_song(vector<vector<vector<int>>> song) {
	double max = 0;
	for (unsigned int i = 0; i < song.size(); i++) {
		if (eval_part(song[i]) > max) {
			max = eval_part(song[i]);
		}
		cout << "パートの難易度" << eval_part(song[i]) << "ポイント" << endl;
	}
	cout << "楽曲の難易度" << max << "ポイント" << endl;
	return max;
}

// 文字列を文字 delimiter で区切って文字列の配列 vector<string> にして
// 返す関数．コンマやタブ区切り形式のテキスト一行を処理するのに使用する．
vector<string> split(const string & input, char delim) {
    std::istringstream stream(input);
    string field;
    vector<string> result;
    while (std::getline(stream, field, delim)) {
        result.push_back(field);
    }
    return result;
}

int main(int argc, char * argv[]) {
	string filename;
	if ( !(argc > 1) ) {
		cerr << "Chord file name is requested." << endl;
		return EXIT_FAILURE;
	} else {
		filename = string(argv[1]);
	}

	ifstream chordfile(filename, std::ios::in);
	if ( !chordfile ) {
		cerr << "オープン失敗" << endl;
		return EXIT_FAILURE;
	}

	vector<vector<vector<int>>> song;
	
    string line;
    while (getline(chordfile, line)) {
    	if (line.size() == 0)
    		continue;
        vector<string> strvec = split(line, '#');
        if (strvec.size() > 1 and strvec[1].starts_with("part")) {
        	song.push_back(vector<vector<int>>());
        }
        if (strvec[0].size() == 0)
        	continue;
        if (song.size() == 0)
        	song.push_back(vector<vector<int>>());
        auto & part = song.back();
        strvec = split(strvec[0], ',');
        if (strvec.size() == 0)
        	continue;
        part.push_back(vector<int>());
        for(const auto & tmp : strvec) {
        	int n = stoi(tmp);
        	part.back().push_back(n);
        }
    }
    chordfile.close();

    cout << "reading file finished." << endl << flush;

	cout << "part " << endl;
	for(const auto & part : song) {
		for(const auto & chord : part) {
			auto note_itr  = chord.begin();
			cout << *note_itr;
			if ( chord.size() > 1) {
				for( ; note_itr != chord.end(); ++note_itr) {
					cout << ", " << *note_itr;
				}
			}
			cout << endl;
		}
		cout << endl;
	}

	eval_song(song);
}
