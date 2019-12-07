#include <iostream>
#include <queue>
using namespace std;
struct Node {
	int id;
	int dist;
	Node(int id, int dist) : id(id), dist(dist) {}
};

struct cmp {
	bool operator()(Node u, Node v){
		return u.dist > v.dist;
	}
};

int main()
{
	priority_queue<Node, vector<Node>, cmp> pq;
	pq.push(Node(0, 3));
	pq.push(Node(1, 7));
	pq.push(Node(2, 11));
	pq.push(Node(3, 4));
	pq.push(Node(4, 6));
	pq.push(Node(5, 8));
	pq.push(Node(2, 10));

	while(!pq.empty()) {
		cout << pq.top().id <<' ' << pq.top().dist << endl;
		pq.pop();
	}
	return 0;
}
