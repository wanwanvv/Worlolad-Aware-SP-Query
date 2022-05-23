#pragma once
#include <vector>
#include <queue>
#include <cstdlib>
#include <assert.h>
#include <ctime>
#include <cstdio>
#include <cassert>
#include <limits.h>
#include <math.h>
#include <queue>
#include "node.h"

using namespace std;

/*
A tree is a tree decomposition. The procedure of building index,
converting Euler sequence, and building ST table are in this class.
*/
class tree
{
public:
	tree() {

	}
	//Construct function. Connect all nodes to build a tree.
	tree(vector<node>& nodes, const vector<int>& level, const bool directed) {
		numOfNodes = nodes.size();
		for (int i = 0; i < numOfNodes; i++) {
			if (directed) {
				nodes[i].setParent_directed(nodes, level, nodes[i].neighbor);
			}
			else {
				nodes[i].setParent(nodes, level, nodes[i].neighbor);
			}
			
			if (!nodes[i].parent) {
				root = &nodes[i];
				rootNum = i;
				root->curNum = rootNum;
				root->height = 1;
				root->level = level[rootNum];
			}
		}
		if (!root) {
			cout << "The tree decomposition is wrong!" << endl;
		}
		setHeightOfEachNode(root, 1);
	}

	//Calculate height of each node in tree decomposition.
	void setHeightOfEachNode(node* curNode, int height) {
		if (!curNode)
			return;
		curNode->height = height;
		for (int i = 0; i < curNode->child.size(); i++) {
			curNode->child[i]->height = height + 1;
			setHeightOfEachNode(curNode->child[i], height + 1);
		}
	}

	//Find all the ancestors of each node. 
	void allAnsGenerator() {
		node* curNode = root;
		curNode->allAns.resize(0);//There is no ancestor of the root node.
		queue<node*> nodeQueue;
		for (int i = 0; i < curNode->child.size(); i++) {
			nodeQueue.push(curNode->child[i]);
		}
		while (!nodeQueue.empty()) {
			curNode = nodeQueue.front();
			curNode->calAllANS();
			nodeQueue.pop();
			for (int i = 0; i < curNode->child.size(); i++) {
				nodeQueue.push(curNode->child[i]);
			}
		}
	}

	//Build index for each node.
	void createLabel(vector<node>& nodes, node* curNode, const bool& directed, const vector<vector<int>>& dis_dijkstra) {
		allAnsGenerator();
		if (!curNode)
			curNode = root;
		queue<node*> nodeQueue;
		nodeQueue.push(curNode);
		while (!nodeQueue.empty()) {
			node* calNode = nodeQueue.front();
			nodeQueue.pop();
			if (directed) {
				calNode->calDis_directed(nodes, dis_dijkstra);
			}
			else {
				calNode->calDis(nodes);
			}
			for (int i = 0; i < calNode->child.size(); i++) {
				nodeQueue.push(calNode->child[i]);
			}
		}
	}

	//Get height of the tree decomposition.
	int getHeight(node* curNode) {
		if (curNode->child.size() == 0)
			return 1;
		int height = -1;
		for (int i = 0; i < curNode->child.size(); i++) {
			int h = getHeight(curNode->child[i]) + 1;
			height = height < h ? h : height;
		}
		return height;
	}

	//Calculate the number of children of each node.
	void calChildren(vector<node>& nodes) {
		for (int i = 0; i < numOfNodes; i++) {
			node* curNode = nodes[i].parent;
			while (curNode) {
				curNode->numOfChildren++;
				curNode = curNode->parent;
			}
		}
		for (int i = 0; i < numOfNodes; i++) {//We set the number of children contains the node itself.
			nodes[i].numOfChildren++;
		}
	}

	//Calculate query cost for each node which is introduced in our paper.
	void calQueryCost(vector<node>& nodes) {
		calChildren(nodes);
		node* p = root;
		while (p->child.size() == 1) {
			p = p->child.front();
		}
		int twoChildrenHeight = p->height;
		for (int i = 0; i < numOfNodes; i++) {
			node* curNode = nodes[i].parent;
			int totalAnsWidth = 0;
			//Calculate width of nodes generated by pos, i.e., the size of neighbors.
			while (curNode) {
				totalAnsWidth += curNode->pos.size() + 1;
				curNode = curNode->parent;
			}
			//minus the nodes itself in Ans
			int numOfAns = nodes[i].height - 1;
			if (numOfAns > 0) {
				if (nodes[i].height > twoChildrenHeight) {
					nodes[i].queryCost = numOfAns / (float)numOfNodes + nodes[i].numOfChildren /
						(float)numOfNodes + (numOfNodes - numOfAns - nodes[i].numOfChildren) /
						(float)numOfNodes * totalAnsWidth / (float)numOfAns;
				}
				else {
					nodes[i].queryCost = 1;
				}
			}
			else {
				nodes[i].queryCost = nodes[i].numOfChildren / (float)numOfNodes;
			}
		}
	}

	//Output query cost of each node.
	void outputQueryCost(const char* file, const vector<node>& nodes) {
		ofstream ofs(file);
		for (int i = 0; i < numOfNodes; i++) {
			ofs << i << " " << nodes[i].queryCost << endl;
		}
		ofs.close();
	}

	//It is a depth-first traversal to build Euler sequence.
	inline void dfs(node* p)
	{
		pos[p->curNum] = dfn;
		Euler[dfn++] = p->curNum;
		for (int i = 0; i < p->child.size(); i++)
		{
			dfs(p->child[i]);
			Euler[dfn++] = p->curNum;
		}
	}

	//Build ST table.
	inline void createST(vector<node>& nodes)
	{
		//Initialize.
		Euler.resize(2 * numOfNodes + 1);
		pos.assign(numOfNodes, 0);
		dep.resize(numOfNodes);
		sparseTable.resize(2 * numOfNodes + 1);
		for (int i = 0; i < 2 * numOfNodes + 1; i++) {
			sparseTable[i].resize(log2(2 * numOfNodes + 1) + 1);
		}
		dep_MAX = 0;
		for (int i = 0; i < numOfNodes; i++) {
			dep[i] = nodes[i].height;
			if (dep[i] > dep[dep_MAX]) {
				dep_MAX = i;
			}
		}

		dfs(root);
		for (int i = 0; i < dfn; i++)
		{
			sparseTable[i][0] = Euler[i];
		}
		for (int j = 0; j < log2(2 * numOfNodes + 1); j++)
		{
			for (int i = 0; i + (1 << (j + 1)) < dfn; i++)
			{
				if (dep[sparseTable[i][j]] < dep[sparseTable[i + (1 << j)][j]])sparseTable[i][j + 1] = sparseTable[i][j];
				else sparseTable[i][j + 1] = sparseTable[i + (1 << j)][j];
			}
		}

		//The following code is used to build ST table with space complexity of O(n).
		EulerLen = 2 * numOfNodes + 1;
		blockSize = log2(EulerLen) / 2;
		globalST.resize(EulerLen / blockSize + 1);

		//Build ST table for query between blocks.
		for (int i = 0; i < globalST.size(); i++) {
			globalST[i].resize(log2(globalST.size()) + 1);
		}
		for (int i = 0; i < globalST.size() - 1; i++)
		{
			int m = Euler[i * blockSize];
			int n = Euler[(i + 1) * blockSize - 1];
			if (i == globalST.size() - 1) {
				n = Euler.back();
			}
			globalST[i][0] = search_LCA_ST(m, n);
		}

		for (int j = 0; j < log2(globalST.size()) + 1; j++)
		{
			for (int i = 0; i + (1 << (j + 1)) < globalST.size() - 2; i++)
			{
				if (dep[globalST[i][j]] < dep[globalST[i + (1 << j)][j]])globalST[i][j + 1] = globalST[i][j];
				else globalST[i][j + 1] = globalST[i + (1 << j)][j];
			}
		}

		//Build ST table for inside blocks.
		insideST.resize(EulerLen / blockSize);
		for (int i = 0; i < insideST.size(); i++) {
			if (i != insideST.size() - 1) {
				insideST[i].resize(blockSize);
			}
			else
				insideST[i].resize(blockSize + 1);
			for (int j = 0; j < insideST[i].size(); j++) {
				insideST[i][j].resize(log2(insideST[i].size()) + 1);
			}
			for (int j = 0; j < insideST[i].size(); j++)
			{
				int location = i * blockSize;
				insideST[i][j][0] = Euler[location + j];
			}
			for (int k = 0; k < log2(insideST[i].size() + 1); k++)
			{
				for (int j = 0; j + (1 << (k + 1)) < insideST[i].size() + 1; j++)
				{
					if (dep[insideST[i][j][k]] < dep[insideST[i][j + (1 << k)][k]])insideST[i][j][k + 1] = insideST[i][j][k];
					else insideST[i][j][k + 1] = insideST[i][j + (1 << k)][k];
				}
			}
		}
	}

	//Search LCA of (x,y) using 0(n) ST table.
	int search_LCA_ST_On(int x, int y) {
		int ans = 0;
		if (pos[x] > pos[y])swap(x, y);
		int l1 = pos[x], l2 = pos[y];
		int xBlock = l1 / blockSize;
		int yBlock = l2 / blockSize;
		int l1_Location = l1;//The position of l1 inside block.
		if (xBlock != 0) {
			l1_Location = l1 % (xBlock * blockSize);
		}
		int l2_Location = l2;////The position of l2 inside block.
		if (yBlock != 0) {
			l2_Location = l2 % (yBlock * blockSize);
		}
		if (xBlock == yBlock) {//If the two block are the same, return directly.
			return search_ST_Block(insideST[xBlock], l1_Location, l2_Location);
		}
		int ans1 = search_ST_Block(globalST, xBlock + 1, yBlock - 1);//Search LCA in the block between two blocks.
		int ans2 = search_ST_Block(insideST[xBlock], l1_Location, blockSize - 1);//Search LCA in block xBlock.
		int ans3 = search_ST_Block(insideST[yBlock], 0, l2_Location);//Search LCA in block yBlock.

		//Calculate the smallest one.
		ans = (dep[ans1] < dep[ans2]) ? ans1 : ans2;
		ans = (dep[ans] < dep[ans3]) ? ans : ans3;
		return ans;
	}

	//Search LCA by using O(nlogn) ST table.
	int search_LCA_ST(int x, int y) {
		int ans = 0;
		if (pos[x] > pos[y])swap(x, y);
		int l1 = pos[x], l2 = pos[y];
		int len = log2(l2 - l1 + 1);
		if (dep[sparseTable[l1][len]] < dep[sparseTable[l2 - (1 << len) + 1][len]])ans = sparseTable[l1][len];
		else ans = sparseTable[l2 - (1 << len) + 1][len];
		return ans;
	}

	//Search LCA in an inside block where l1 < l2.
	int search_ST_Block(const vector<vector<int>>& ST, int l1, int l2) {
		//Process search between blocks where l1 = l2 + 1.
		if (l1 > l2) {
			return dep_MAX;
		}
		if (l1 == l2) {
			return ST[l1][0];
		}
		int ans = 0;
		int len = log2(l2 - l1 + 1);
		if (dep[ST[l1][len]] < dep[ST[l2 - (1 << len) + 1][len]])ans = ST[l1][len];
		else ans = ST[l2 - (1 << len) + 1][len];
		return ans;
	}

	//Query dis(m, n) by using O(nlogn) ST table in indirected graph.
	int query_ST(vector<node>& nodes, int& m, int& n) {
		int ans = search_LCA_ST(m, n);
		vector<int>& dis_m = nodes[m].dis;
		vector<int>& dis_n = nodes[n].dis;
		vector<int>& curPos = nodes[ans].pos;

		//Return distance in O(1) time if there is an ancestor-child relationship.
		if (ans == m) {
			return dis_n[curPos.back()];
		}
		if (ans == n) {
			return dis_m[curPos.back()];
		}

		int distance{ INT32_MAX };
		for (int i = 0; i < curPos.size(); i++) {
			int curDis = dis_m[curPos[i]] + dis_n[curPos[i]];
			if (distance > curDis) {
				distance = curDis;
			}
		}
		return distance;
	}

	//Query dis(m, n) by using O(nlogn) ST table in directed graph.
	int query_ST_directed(vector<node>& nodes, int& m, int& n) {
		int ans = search_LCA_ST(m, n);
		vector<int>& dis_m = nodes[m].dis_out;
		vector<int>& dis_n = nodes[n].dis_in;
		vector<int>& curPos = nodes[ans].pos;

		//Return distance in O(1) time if there is an ancestor-child relationship.
		if (ans == m) {
			return dis_n[curPos.back()];
		}
		if (ans == n) {
			return dis_m[curPos.back()];
		}

		int distance{ INT32_MAX };
		for (int i = 0; i < curPos.size(); i++) {
			int curDis = dis_m[curPos[i]] + dis_n[curPos[i]];
			if (distance > curDis) {
				distance = curDis;
			}
		}
		return distance;
	}

	//Query dis(m, n) by using O(n) ST table in directed graph.
	int query_ST_On_directed(vector<node>& nodes, int& m, int& n) {
		int ans = search_LCA_ST_On(m, n);
		vector<int>& dis_m = nodes[m].dis_out;
		vector<int>& dis_n = nodes[n].dis_in;
		vector<int>& curPos = nodes[ans].pos;
		//Return distance in O(1) time if there is an ancestor-child relationship.
		if (ans == m) {
			return dis_n[curPos.back()];
		}
		if (ans == n) {
			return dis_m[curPos.back()];
		}

		int distance{ INT32_MAX };
		for (int i = 0; i < curPos.size(); i++) {
			int curDis = dis_m[curPos[i]] + dis_n[curPos[i]];
			if (distance > curDis) {
				distance = curDis;
			}
		}
		return distance;
	}

	//Query dis(m, n) by using O(n) ST table in indirected graph.
	int query_ST_On(vector<node>& nodes, int& m, int& n) {
		int ans = search_LCA_ST_On(m, n);
		vector<int>& dis_m = nodes[m].dis;
		vector<int>& dis_n = nodes[n].dis;
		vector<int>& curPos = nodes[ans].pos;
		//Return distance in O(1) time if there is an ancestor-child relationship.
		if (ans == m) {
			return dis_n[curPos.back()];
		}
		if (ans == n) {
			return dis_m[curPos.back()];
		}

		int distance{ INT32_MAX };
		for (int i = 0; i < curPos.size(); i++) {
			int curDis = dis_m[curPos[i]] + dis_n[curPos[i]];
			if (distance > curDis) {
				distance = curDis;
			}
		}
		return distance;
	}

	//Output ST table.
	void save_st_binary(const char* st_filename, const char* st_block_filename) {
		//Output O(nlogn) ST table.
		ofstream ofsST(st_filename, ios::binary | ios::out);
		for (int i = 0; i < sparseTable.size(); i++) {
			for (int j = 0; j < sparseTable[i].size(); j++) {
				ofsST.write((const char*)&(sparseTable[i][j]), sizeof(sparseTable[i][j]));
			}
		}
		for (int i = 0; i < pos.size(); i++) {
			ofsST.write((const char*)&pos[i], sizeof(pos[i]));
		}
		for (int i = 0; i < dep.size(); i++) {
			ofsST.write((const char*)&dep[i], sizeof(dep[i]));
		}
		ofsST.close();

		//Output O(n) ST table.
		ofstream ofsST_block(st_block_filename, ios::binary | ios::out);
		for (int i = 0; i < globalST.size(); i++) {
			for (int j = 0; j < globalST[i].size(); j++) {
				ofsST_block.write((const char*)&(globalST[i][j]), sizeof(globalST[i][j]));
			}
		}
		for (int i = 0; i < insideST.size(); i++) {
			for (int j = 0; j < insideST[i].size(); j++) {
				for (int k = 0; k < insideST[i][j].size(); k++) {
					ofsST_block.write((const char*)&(insideST[i][j][k]), sizeof(insideST[i][j][k]));
				}
			}
		}
		for (int i = 0; i < pos.size(); i++) {
			ofsST_block.write((const char*)&pos[i], sizeof(pos[i]));
		}
		for (int i = 0; i < dep.size(); i++) {
			ofsST_block.write((const char*)&dep[i], sizeof(dep[i]));
		}
		ofsST_block.close();
	}

	//Output index.
	void save_labels(const char* save_filename, const vector<node>& nodes, const bool directed) {
		ofstream ofs(save_filename, ios::binary | ios::out);
		int numOfNodes = nodes.size();
		ofs.write((const char*)&numOfNodes, sizeof(numOfNodes));
		//Output dis array.
		if (directed) {
			for (int v = 0; v < numOfNodes; ++v) {
				int disSize = nodes[v].dis_in.size();
				ofs.write((const char*)&disSize, sizeof(disSize));
				for (int i = 0; i < disSize; ++i) {
					ofs.write((const char*)&nodes[v].dis_in[i], sizeof(nodes[v].dis_in[i]));
				}
				disSize = nodes[v].dis_out.size();//In fact, the size of dis_in and dis_out is equal.
				ofs.write((const char*)&disSize, sizeof(disSize));
				for (int i = 0; i < disSize; ++i) {
					ofs.write((const char*)&nodes[v].dis_out[i], sizeof(nodes[v].dis_out[i]));
				}
			}
		}
		else {
			for (int v = 0; v < numOfNodes; ++v) {
				int disSize = nodes[v].dis.size();
				ofs.write((const char*)&disSize, sizeof(disSize));
				for (int i = 0; i < disSize; ++i) {
					ofs.write((const char*)&nodes[v].dis[i], sizeof(nodes[v].dis[i]));
				}
			}
		}
		
		//Output pos array.
		for (int v = 0; v < numOfNodes; ++v) {
			int posSize = nodes[v].pos.size();
			//We don't need to save the pos array of leaves. 
			if (nodes[v].child.size() == 0) {
				posSize = 0;
			}
			ofs.write((const char*)&posSize, sizeof(posSize));
			if (posSize != 0) {
				for (int i = 0; i < nodes[v].pos.size(); ++i) {
					ofs.write((const char*)&nodes[v].pos[i], sizeof(nodes[v].pos[i]));
				}
			}
		}
		ofs.close();
	}

	node* getRoot() {
		return root;
	}


private:
	vector<int> pos;//Save the position of each node in Euler sequence.
	vector<int> dep;//Save the height of each node in tree decomposition.
	int dep_MAX;//Record the maximum depth for O(n) ST table.
	vector<int> Euler;//Save the ID of each node in Euler sequence.
	vector<vector<int>> sparseTable;//O(nlogn) ST table
	vector<vector<int>> globalST;//ST table between blocks
	vector<vector<vector<int>>> insideST;//ST table inside block
	vector<float> avgQueryCost;
	int EulerLen;//length of Euler sequence
	int blockSize;//The size of blocks in O(n) ST table.
	int dfn = 0;//Record the number of nodes traversed in dfs.
	node* root = NULL;
	int rootNum;//ID of root node
	int numOfNodes;
};
