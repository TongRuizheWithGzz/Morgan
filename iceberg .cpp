#include<fstream>
#include<iostream>
#include<sstream>
#include<string>
#include<vector>
#include<random>
using namespace std;





struct TimePoint {
	int hour = 9, minute = 0;
	TimePoint(int hour) :hour(hour) {}
	TimePoint() = default;
};
inline int mark(TimePoint& timePoint) {
	return 60 * (timePoint.hour - 9) + timePoint.minute;
}
TimePoint operator+(TimePoint& timePoint, int min) {
	timePoint.minute += min;
	if (timePoint.minute >= 60) {
		timePoint.minute -= 60;
		timePoint.hour += 1;
	}
	return timePoint;
}


struct line {
	TimePoint t;
	int amount = 0;
	double price;
};
inline void readline(string& li, line& nextline) {
	string temp;//useless
	int pr, u;
	for (char &k : li)
		if (k == ',' || k == ':')
			k = ' ';
	istringstream reader(li);
	reader >> temp
		>> nextline.t.hour >> nextline.t.minute >> u
		>> nextline.price >> nextline.amount;
}




class Iceberg
{
private:
	struct singleOrder {
		double amount = 0;
		double price = 0;
	};
	int quantity = 0;
	vector<singleOrder> proportion = vector<singleOrder>(105);
	int getRandom();
public:
	void setQuantity(int amount) { quantity = amount; }
	void output_distribute(ostream& out);
	void input_distribute(ifstream& in);
};

int Iceberg::getRandom()
{
	random_device rd;     // only used once to initialise (seed) engine
	mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
	uniform_int_distribution<int> uni(0,3); // guaranteed unbiased
	auto random_integer = uni(rng);
	return random_integer;
}

void Iceberg::output_distribute(ostream & out)
{
	TimePoint time(9);
	double sum = 0;
	for (int i = 0; i < 105; ++i) {
		int min = time.minute + getRandom();
		if (min < 10)
			cout << time.hour << ":0" << min
			<< " quantity:" << int(proportion[i].amount * quantity)
			<< " price: " << proportion[i].price << endl;
		else
			cout << time.hour << ":" << min
			<< " quantity:" << int(proportion[i].amount * quantity)
			<< " price: " << proportion[i].price << endl;
		time = time + 4;
		sum += int(proportion[i].amount * quantity)*proportion[i].price;
	}
	cout << "Average price: " << sum / quantity << endl;
}

void Iceberg::input_distribute(ifstream & in)
{
	int sum_amount = 0;
	string temp;
	line current;
	getline(in, temp);
	while (getline(in, temp)) {
		if (temp[0] == '*')
			continue;
		readline(temp, current);
		sum_amount += current.amount;
		proportion[mark(current.t) / 4].amount += current.amount;
		proportion[mark(current.t) / 4].price += current.price;
	}
	for (singleOrder& p : proportion) {
		p.amount /= sum_amount;
	}
}

int main() {
	int quantity = 0;
	ifstream inFile;
	inFile.open("600690.iceberg.csv");
	cout << "Quantity of order?";
	cin >> quantity;
	Iceberg iceberg;
	iceberg.setQuantity(quantity);
	iceberg.input_distribute(inFile);
	iceberg.output_distribute(cout);
	inFile.close();

}