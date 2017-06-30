#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include<list>
#include <set>
#include<algorithm>
using namespace std;


enum OrderSide { buy = 0, sell };
enum QuoteSide { ask = 0, bid };

struct Order
{
	double price;
	int quantity;
	OrderSide side;
	int orderId;
	string symbol;
	map<string, int> distribute;  //Record current distribute on each exchange. string->exchange,int->quatity
	map<string, pair<int, double>> filled;//Record the filled part. string->exchange,int->quatity,double->price
	bool isFilled()const{ return quantity == 0; }//When the quantity reduce to 0, it is filled.
	bool outed = false;
};

struct Quote
{
	double price;
	int size;
	QuoteSide side;
	string exchange;
	string symbol;
};

struct level1Book
{
	struct currentSituation
	{
		int size;
		double price;
	};
	map<string, currentSituation> book;//string->exchange,curreSituation->current size and current price.
};

class OrderBook
{

private:
	void fillOrder(Quote *, Order *);//Fill the order according to the quote.

public:
	list<Order *> orderBook;//Record the order.
	void outputInfo(Order *);//output the info of the order
	void addOrder(Order *order) { orderBook.push_back(order); }
	void fillOrder(Quote *);//fill the order through the orderbook.
	
};
bool isCrossed(const Quote *quote, const Order *order)
{
	if (quote->side == bid)
		return quote->price >= order->price;
	else
		return quote->price <= order->price;
}
class Sort
{
private:
	map<string, level1Book> bidBooks;//string-?symbol
	map<string, level1Book> askBooks;//string->symbol
	map<string, OrderBook> buyOrderBooks;//string->symbol
	map<string, OrderBook> sellOrderBooks;//string->symbol
	void updateBook(const Quote*, level1Book&);
	void splitOrder(Order *, const level1Book&);
public:
	Quote *readQuote(istream& in);
	void updateLevelBook(const Quote*);
	void readOrder(istream& in);
	void splitOrder(Order *);
	void dealWithQuote(Quote *);
	~Sort();
};



void Sort::updateBook(const Quote *quote, level1Book &book1)
{
	book1.book[quote->exchange].price = quote->price;
	book1.book[quote->exchange].size = quote->size;
}

void Sort::splitOrder(Order *order, const level1Book &book)
{
	double sum = 0;
	for (auto &i : book.book)
		sum += i.second.size;
	if (sum == 0)
		return;
	for (auto& i : book.book)
		order->distribute[i.first] = order->quantity*(i.second.size / sum);
}

Quote *Sort::readQuote(istream & in)
{
	Quote *quote=new Quote();
	string side;
	char separator;
	while (in >> separator&&separator != ',')
		quote->exchange += separator;
	while (in >> separator&&separator != ',')
		side += separator;
	if (side == "Bid")
		quote->side = bid;
	else
		quote->side = ask;
	while (in >> separator&&separator != ',')
		quote->symbol += separator;
	in >> quote->size;
	in >> separator;
	in >> quote->price;
	if (!in)
		return nullptr;
	return quote;
}

void Sort::updateLevelBook(const Quote *quote)
{
	if (quote->side == ask)
		updateBook(quote, askBooks[quote->symbol]);
	else
		updateBook(quote, bidBooks[quote->symbol]);
}

void Sort::readOrder(istream & in)
{
	Order *order=new Order();
	string side;
	char separator;
	in >> order->orderId;
	in >> separator;
	while (in >> separator&&separator != ',')
		side += separator;
	if (side == "Buy")
		order->side = buy;
	else
		order->side = sell;
	while (in >> separator&&separator != ',')
		order->symbol += separator;
	in >> order->price;
	in >> separator;
	in >> order->quantity;
	if (in) {
		if (order->side == buy)
			buyOrderBooks[order->symbol].addOrder(order);
		else
			sellOrderBooks[order->symbol].addOrder(order);
	}
}

void Sort::splitOrder(Order *order)
{
	if (order->side == buy)
		splitOrder(order, askBooks[order->symbol]);
	else
		splitOrder(order, bidBooks[order->symbol]);
}

void Sort::dealWithQuote(Quote *quote)
{

	
	if (quote->side == ask) 
		buyOrderBooks[quote->symbol].fillOrder(quote);
	else
		sellOrderBooks[quote->symbol].fillOrder(quote);
	if (quote->size != 0)
		updateLevelBook(quote);
	
	OrderBook &orderbook = quote->size ? sellOrderBooks[quote->symbol] : buyOrderBooks[quote->symbol];
	for (auto iter = orderbook.orderBook.begin(); iter != orderbook.orderBook.end(); iter++)
		splitOrder(*iter);
	delete quote;
}

Sort::~Sort()
{
	for (auto iter = buyOrderBooks.begin(); iter != buyOrderBooks.end(); iter++) {
		for (auto it = iter->second.orderBook.begin(); it != iter->second.orderBook.end(); it++)
			if (*it)
				delete *it;
	}
	for (auto iter = sellOrderBooks.begin(); iter != sellOrderBooks.end(); iter++) {
		for (auto it = iter->second.orderBook.begin(); it != iter->second.orderBook.end(); it++)
			if (*it)
				delete *it;
	}
}


int minNumber(int a, int b) {
	return a < b ? a : b;
}
void OrderBook::fillOrder(Quote *quote, Order *order)
{
	if (order->isFilled()) {
		if (!order->outed) {
			outputInfo(order);
			order->outed = true;
		}
		return;
	}
	if (isCrossed(quote, order)) {
		int minSize = minNumber(quote->size, order->quantity);
		quote->size -= minSize;
		order->quantity -= minSize;
		order->filled[quote->exchange].first += minSize;
		order->filled[quote->exchange].second += quote->price*minSize;
	}

}

void OrderBook::outputInfo(Order *order)
{
	cout << "orderId: " << order->orderId << " " << "symbol: " << order->symbol << " ";
	for (auto i : order->filled) {
		cout << i.first << ":" << "quatity: " << i.second.first << " " << "average price: " << i.second.second / i.second.first << endl;

	}
}

void OrderBook::fillOrder(Quote *quote)
{
	for (auto iter = orderBook.begin(); iter != orderBook.end(); iter++) {
		fillOrder(quote, *iter);
	}
}
int man()
{
	Sort sort;
	ifstream quoteFile, OrderFile;
	quoteFile.open("Quotes.csv");
	OrderFile.open("Orders.csv");
	string line;
	getline(OrderFile, line);
	while (OrderFile)
		sort.readOrder(OrderFile);
	OrderFile.close();
	getline(quoteFile, line);
	while (quoteFile) {
		Quote *quote = sort.readQuote(quoteFile);
		if (!quote)
			continue;
		sort.dealWithQuote(quote);
	}
	quoteFile.close();
	return 0;
}