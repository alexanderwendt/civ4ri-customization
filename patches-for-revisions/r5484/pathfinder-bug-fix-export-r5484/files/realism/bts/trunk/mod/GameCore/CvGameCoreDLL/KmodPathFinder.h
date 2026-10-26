#pragma once

#include <vector>
#include "FAStarNode.h"

// RI FIX 1 BEGIN
class CvSelectionGroup;
class CvPlot;
// RI FIX 1 END

struct CvPathSettings
{
	CvPathSettings(const CvSelectionGroup* pGroup = 0, int iFlags = 0, int iMaxPath = -1, int iHW = -1);

	CvSelectionGroup* pGroup;
	int iFlags;
	int iMaxPath;
	int iHeuristicWeight;
};

class FAStarNode;

class KmodPathFinder
{
public:
	static void InitHeuristicWeights();
	static int MinimumStepCost(int BaseMoves);

	KmodPathFinder();
	~KmodPathFinder();

	bool ValidateNodeMap(); // Called when SetSettings is used.

	bool GeneratePath(int x1, int y1, int x2, int y2);
	bool GeneratePath(const CvPlot* pToPlot); // just a wrapper for convenience
	FAStarNode* GetEndNode() const { FAssert(end_node); return end_node; } // Note: the returned pointer becomes invalid if the pathfinder is destroyed.
	bool IsPathComplete() const { return end_node; }
	int GetPathTurns() const;
	int GetFinalMoves() const;
	CvPlot* GetPathFirstPlot() const;
	CvPlot* GetPathEndTurnPlot() const;
	void SetSettings(const CvPathSettings& new_settings);
	void SetSettings(const CvSelectionGroup* pGroup, int iFlags = 0, int iMaxPath = -1, int iHW=-1) { SetSettings(CvPathSettings(pGroup, iFlags, iMaxPath, iHW)); }
	void Reset();
// RI FIX 2 BEGIN
	void Clear();
// RI FIX 3 BEGIN
	bool PrepareNodeMap(); // storage only; allowed before final initialization
// RI FIX 3 END
	static KmodPathFinder& ColoredPlotFinder();
	static void ClearColoredPlotFinder();
// RI FIX 2 END

// RI FIX 1 BEGIN
private:
	// Owning buffers must never be shallow-copied.
	KmodPathFinder(const KmodPathFinder&);
	KmodPathFinder& operator=(const KmodPathFinder&);
// RI FIX 1 END

protected:
	void AddStartNode();
	void RecalculateHeuristics();
	bool ProcessNode();
	// RI FIX 9 BEGIN
bool ForwardPropagate(FAStarNode* head, int cost_delta);
// RI FIX 9 END
	typedef std::vector<FAStarNode*> OpenList_t;

	struct OpenList_sortPred
	{
		bool operator()(const FAStarNode* &left, const FAStarNode* &right);
	};

	// RI FIX 4 BEGIN
FAStarNode& GetNode(int x, int y)
    {
        const size_t index = (size_t)y * map_width + x;
        return node_blocks[index / NODE_BLOCK_SIZE][index % NODE_BLOCK_SIZE];
    }
    enum { NODE_BLOCK_SIZE = 256 };
    void ReleaseNodeMap();
// RI FIX 4 END
	// RI FIX 4 BEGIN
FAStarNode** node_blocks;
    size_t node_block_count;
// RI FIX 4 END
	OpenList_t open_list;

	int dest_x, dest_y;
	int start_x, start_y;
	FAStarNode* end_node;
	CvPathSettings settings;
	int map_width, map_height;

	static int admissible_scaled_weight;
	static int admissible_base_weight;
};
