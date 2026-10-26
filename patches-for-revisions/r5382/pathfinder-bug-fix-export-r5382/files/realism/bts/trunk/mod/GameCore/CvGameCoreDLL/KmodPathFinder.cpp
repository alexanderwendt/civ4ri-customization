

#include "CvGameCoreDLL.h"
#include "KmodPathFinder.h"
#include "CvGameCoreUtils.h"

// RI FIX 2 BEGIN
// Non-owning registry: map teardown must not construct or resurrect a static finder.
static KmodPathFinder* riColoredPlotFinder = NULL;
// RI FIX 2 END
int KmodPathFinder::admissible_scaled_weight = 1;
int KmodPathFinder::admissible_base_weight = 1;

CvPathSettings::CvPathSettings(const CvSelectionGroup* pGroup, int iFlags, int iMaxPath, int iHW)
	: pGroup(const_cast<CvSelectionGroup*>(pGroup)), iFlags(iFlags), iMaxPath(iMaxPath), iHeuristicWeight(iHW)
// I'm really sorry about the const_cast. I can't fix the const-correctness of all the relevant functions,
// because some of them are dllexports. The original code essentially does the same thing anyway, with void* casts.
{
	// empty. (there use to be something here, but we don't need it anymore.)
}

void KmodPathFinder::InitHeuristicWeights()
{
	admissible_base_weight = GC.getMOVE_DENOMINATOR()/2;
	admissible_scaled_weight = GC.getMOVE_DENOMINATOR()/2;
	for (int r = 0; r < GC.getNumRouteInfos(); r++)
	{
		const CvRouteInfo& kInfo = GC.getRouteInfo((RouteTypes)r);
		int iCost = kInfo.getMovementCost();
		for (int t = 0; t < GC.getNumTechInfos(); t++)
		{
			if (kInfo.getTechMovementChange(t) < 0)
				iCost += kInfo.getTechMovementChange(t);
		}
		admissible_base_weight = std::min(admissible_base_weight, iCost);
		admissible_scaled_weight = std::min(admissible_scaled_weight, kInfo.getFlatMovementCost());
	}
}

int KmodPathFinder::MinimumStepCost(int BaseMoves)
{
	return std::max(1, std::min(admissible_base_weight, BaseMoves * admissible_scaled_weight));
}

bool KmodPathFinder::OpenList_sortPred::operator()(const FAStarNode* &left, const FAStarNode* &right)
{
	return left->m_iTotalCost < right->m_iTotalCost;
}

//
KmodPathFinder::KmodPathFinder() :
// RI FIX 1 BEGIN
	dest_x(-1), dest_y(-1), start_x(-1), start_y(-1),
// RI FIX 1 END
	end_node(0),
	map_width(0),
	map_height(0),
// RI FIX 4 BEGIN
	node_blocks(0), node_block_count(0)
// RI FIX 4 END
{
	// Unfortunately, the pathfinder is constructed before the map width and height are determined.

	// Ideally the pathfinder would be initialised with a given CvMap, and then not refer to any global objects.
	// But that is not easy to do with the current code-base. Instead I'll just check the pathfinder settings
	// at particular times to make sure the map hasn't changed.

}

KmodPathFinder::~KmodPathFinder()
{
// RI FIX 2 BEGIN
    if (riColoredPlotFinder == this)
        riColoredPlotFinder = NULL; // later CvMap destruction must not touch this object
// RI FIX 2 END
// RI FIX 4 BEGIN
	ReleaseNodeMap();
// RI FIX 4 END
}

// RI FIX 1 BEGIN - validate dimensions, preserve allocation on failure, reset after resize.
bool KmodPathFinder::ValidateNodeMap()
{
    PROFILE_FUNC();
    if (!GC.getGameINLINE().isFinalInitialized())
        return false;
    // RI FIX 3 BEGIN - gameplay gate belongs here, not in the storage allocator.
    return PrepareNodeMap();
}

bool KmodPathFinder::PrepareNodeMap()
{
    // RI FIX 3 END
    const int width = GC.getMapINLINE().getGridWidthINLINE();
    const int height = GC.getMapINLINE().getGridHeightINLINE();
    if (width <= 0 || height <= 0 || width > MAX_INT / height ||
        (size_t)width > ((size_t)-1) / sizeof(FAStarNode) / (size_t)height)
    {
        open_list.clear();
        end_node = NULL;
        settings.pGroup = NULL;
        return false;
    }
// RI FIX 4 BEGIN
    if (!node_blocks || map_width != width || map_height != height)
    {
        // Each node block is about 22 KiB on BTS; node addresses never move.
        // Free obsolete storage first: dimensions changed, so its paths are unusable.
        ReleaseNodeMap();
        const size_t count = (size_t)width * height;
        const size_t blocks = (count + NODE_BLOCK_SIZE - 1) / NODE_BLOCK_SIZE;
        node_blocks = (FAStarNode**)calloc(blocks, sizeof(FAStarNode*));
        if (!node_blocks)
        {
            settings.pGroup = NULL;
            return false;
        }
        node_block_count = blocks;
        for (size_t i = 0; i < blocks; ++i)
        {
            node_blocks[i] = (FAStarNode*)calloc(NODE_BLOCK_SIZE, sizeof(FAStarNode));
            if (!node_blocks[i])
            {
                ReleaseNodeMap(); // also frees partially allocated block directories
                settings.pGroup = NULL;
                return false;
            }
        }
        map_width = width;
        map_height = height;
    }
// RI FIX 4 END
    return true;
}
// RI FIX 1 END

bool KmodPathFinder::GeneratePath(int x1, int y1, int x2, int y2)
{
	PROFILE_FUNC();

// RI FIX 9 BEGIN
    // Release builds must reject invalid coordinates before GetNode/pathDestValid.
    end_node = NULL;
    if (!node_blocks || !settings.pGroup || x1 < 0 || x1 >= map_width ||
        y1 < 0 || y1 >= map_height || x2 < 0 || x2 >= map_width ||
        y2 < 0 || y2 >= map_height)
        return false;
// RI FIX 9 END
	FASSERT_BOUNDS(0, map_width , x1, "GeneratePath");
	FASSERT_BOUNDS(0, map_height, y1, "GeneratePath");
	FASSERT_BOUNDS(0, map_width , x2, "GeneratePath");
	FASSERT_BOUNDS(0, map_height, y2, "GeneratePath");

	end_node = NULL;

	if (!settings.pGroup)
		return false;

	if (!pathDestValid(x2, y2, &settings, 0))
		return false;

	if (x1 != start_x || y1 != start_y)
	{
		// Note: it may be possible to salvage some of the old data to get more speed.
		// eg. If the moves recorded on the node match the group,
		// just delete everything that isn't a direct descendant of the new start.
		// and then subtract the start cost & moves off all the remaining nodes.
		Reset(); // but this is easier.
	}
	bool bRecalcHeuristics = false;

	if (dest_x != x2 || dest_y != y2)
		bRecalcHeuristics = true;

	start_x = x1;
	start_y = y1;
	dest_x = x2;
	dest_y = y2;

	if (GetNode(x1, y1).m_bOnStack)
	{
		int iMoves = (settings.iFlags & MOVE_MAX_MOVES) ? settings.pGroup->maxMoves() : settings.pGroup->movesLeft();
		if (iMoves != GetNode(x1,  y1).m_iData1)
		{
			Reset();
			FAssert(!GetNode(x1,  y1).m_bOnStack);
		}
		// Note: This condition isn't actually enough to catch all signifiant changes.
		// We really need to check max moves /and/ moves left /and/ base moves.
		// but I don't feel like doing all that at the moment.
	}

	if (!GetNode(x1,  y1).m_bOnStack)
	{
		AddStartNode();
		bRecalcHeuristics = true;
	}
	//else (not else. maybe start == dest)
	{
		// check if the end plot is already mapped.
		if (GetNode(x2,  y2).m_bOnStack)
			end_node = &GetNode(x2,  y2);
	}

	if (bRecalcHeuristics)
		RecalculateHeuristics();

	while (ProcessNode())
	{
		// nothing
	}

	if (end_node && (settings.iMaxPath < 0 || end_node->m_iData2 <= settings.iMaxPath))
		return true;

	return false;
}

bool KmodPathFinder::GeneratePath(const CvPlot* pToPlot)
{
	// RI FIX 9 BEGIN
if (!settings.pGroup || !pToPlot || !settings.pGroup->plot())
// RI FIX 9 END
		return false;
	return GeneratePath(settings.pGroup->plot()->getX_INLINE(), settings.pGroup->plot()->getY_INLINE(),
		pToPlot->getX_INLINE(), pToPlot->getY_INLINE());
}

int KmodPathFinder::GetPathTurns() const
{
	FAssert(end_node);
	return end_node ? end_node->m_iData2 : 0;
}

int KmodPathFinder::GetFinalMoves() const
{
	FAssert(end_node);
	return end_node ? end_node->m_iData1 : 0;
}

CvPlot* KmodPathFinder::GetPathFirstPlot() const
{
	FAssert(end_node);
	if (!end_node)
		return NULL;

	FAStarNode* node = end_node;

	if (!node->m_pParent)
		return GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);

	while (node->m_pParent->m_pParent)
	{
		node = node->m_pParent;
	}

	return GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);
}

CvPlot* KmodPathFinder::GetPathEndTurnPlot() const
{
	FAssert(end_node);

	FAStarNode* node = end_node;

	FAssert(!node || node->m_iData2 == 1 || node->m_pParent);

	while (node && node->m_iData2 > 1)
	{
		node = node->m_pParent;
	}
	FAssert(node);
	return node ? GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY) : NULL;
}

void KmodPathFinder::SetSettings(const CvPathSettings& new_settings)
{
	// whenever settings are changed, check that we have the right map size.
	if (!ValidateNodeMap())
	{
		FAssertMsg(false, "Failed to validate node map for pathfinder.");
		settings.pGroup = NULL;
		return;
	}

	// some flags are not relevant to pathfinder. We should try to strip those out to avoid unnecessary resets.
	int relevant_flags = ~(MOVE_DIRECT_ATTACK | MOVE_SINGLE_ATTACK | MOVE_NO_ATTACK); // any bar these

	if (settings.pGroup != new_settings.pGroup)
	{
		Reset();
	}
	else if ((settings.iFlags & relevant_flags) != (new_settings.iFlags & relevant_flags))
	{
		// there's one more chance to avoid a reset..
		// If the war flag is the only difference, and we aren't sneak attack ready anyway - then there is effectively no difference.
		relevant_flags &= ~MOVE_DECLARE_WAR;
		if ((settings.iFlags & relevant_flags) != (new_settings.iFlags & relevant_flags)
			|| GET_TEAM(settings.pGroup->getHeadTeam()).AI_isSneakAttackReady())
		{
			Reset();
		}
	}
	settings = new_settings;

	if (settings.iHeuristicWeight < 0)
	{
		if (!settings.pGroup)
		{
			settings.iHeuristicWeight = 1;
		}
		else
		{
			if (settings.pGroup->getDomainType() == DOMAIN_SEA)
			{
				// this assume there are no sea-roads, or promotions to reduce sea movement cost.
				settings.iHeuristicWeight = GC.getMOVE_DENOMINATOR();
			}
			else
			{
				settings.iHeuristicWeight = MinimumStepCost(settings.pGroup->baseMoves());
			}
		}
	}
}

// RI FIX 1 BEGIN - null-safe, overflow-safe reset.
// RI FIX 4 BEGIN - clear blocks without moving node addresses.
void KmodPathFinder::Reset()
{
    for (size_t i = 0; i < node_block_count; ++i)
        memset(node_blocks[i], 0, NODE_BLOCK_SIZE * sizeof(FAStarNode));
    open_list.clear();
    end_node = NULL;
}

void KmodPathFinder::ReleaseNodeMap()
{
    open_list.clear();
    end_node = NULL;
    for (size_t i = 0; i < node_block_count; ++i)
        free(node_blocks[i]);
    free(node_blocks);
    node_blocks = NULL;
    node_block_count = 0;
    map_width = map_height = 0;
}
// RI FIX 4 END
// RI FIX 1 END

void KmodPathFinder::AddStartNode()
{
	FASSERT_BOUNDS(0, map_width , start_x, "KmodPathFinder::AddStartNode");
	FASSERT_BOUNDS(0, map_height, start_y, "KmodPathFinder::AddStartNode");

	// add initial node.
	FAStarNode* start_node = &GetNode(start_x, start_y);
	start_node->m_iX = start_x;
	start_node->m_iY = start_y;
	pathAdd(0, start_node, ASNC_INITIALADD, &settings, 0);
	start_node->m_iKnownCost = 0;

	open_list.push_back(start_node);

	// Note: I'd like to use NO_FASTARLIST as a signal that the node is uninitialised, but unfortunately
	// the default value for m_eFAStarListType is FASTARLIST_OPEN, because FASTARLIST_OPEN == 0.
	// Hence KmodPathFinder uses m_bOnStack to mean the node is connected. m_eFAStarListType is initialised manually.

	start_node->m_eFAStarListType = FASTARLIST_OPEN; // This means nothing. But maybe one day I'll use it.
	start_node->m_bOnStack = true; // This means the node is connected and ready to be used.
}

void KmodPathFinder::RecalculateHeuristics()
{
	// recalculate heuristic cost for all open nodes.
	for (OpenList_t::iterator i = open_list.begin(); i != open_list.end(); ++i)
	{
		int h = settings.iHeuristicWeight * pathHeuristic((*i)->m_iX, (*i)->m_iY, dest_x, dest_y);
		(*i)->m_iHeuristicCost = h;
		(*i)->m_iTotalCost = h + (*i)->m_iKnownCost;
	}
}

bool KmodPathFinder::ProcessNode()
{
	OpenList_t::iterator best_it = open_list.end();
	{
		int iLowestCost = end_node ? end_node->m_iKnownCost : MAX_INT;
		for (OpenList_t::iterator it = open_list.begin(); it != open_list.end(); ++it)
		{
			if ((*it)->m_iTotalCost < iLowestCost &&
				(settings.iMaxPath < 0 || (*it)->m_iData2 <= settings.iMaxPath))
			{
				best_it = it;
				iLowestCost = (*it)->m_iTotalCost;
			}
		}
	}

	// if we didn't find a suitable node to process, then quit.
	if (best_it == open_list.end())
		return false;

	FAStarNode* parent_node = (*best_it);

	// erase the node from the open_list.
	// Note: this needs to be done before pushing new entries, otherwise the iterator will be invalid.
	open_list.erase(best_it);
	parent_node->m_eFAStarListType = FASTARLIST_CLOSED;

	FAssert(&GetNode(parent_node->m_iX, parent_node->m_iY) == parent_node);

	// open a new node for each direction coming off the chosen node.
	for (int i = 0; i < NUM_DIRECTION_TYPES; i++)
	{
		CvPlot* pAdjacentPlot = plotDirection(parent_node->m_iX, parent_node->m_iY, (DirectionTypes)i);
		if (!pAdjacentPlot)
			continue;

		const int& x = pAdjacentPlot->getX_INLINE(); // convenience
		const int& y = pAdjacentPlot->getY_INLINE(); //

		if (parent_node->m_pParent && parent_node->m_pParent->m_iX == x && parent_node->m_pParent->m_iY == y)
			continue; // no need to backtrack.

		//int iPlotNum = GC.getMapINLINE().plotNumINLINE(x, y);
		//FAssert(iPlotNum >= 0 && iPlotNum < GC.getMapINLINE().numPlotsINLINE());

// RI FIX 9 BEGIN
        if (x < 0 || x >= map_width || y < 0 || y >= map_height)
        {
            Reset();
            return false;
        }
// RI FIX 9 END
		FAStarNode* child_node = &GetNode(x, y);
		bool bNewNode = !child_node->m_bOnStack;

		if (bNewNode)
		{
			child_node->m_iX = x;
			child_node->m_iY = y;
			pathAdd(parent_node, child_node, ASNC_NEWADD, &settings, 0);

			if (pathValid_join(parent_node, child_node, settings.pGroup , settings.iFlags))
			{
				// This path to the new node is valid. So we need to fill in the data.
				child_node->m_iKnownCost = MAX_INT;
				child_node->m_iHeuristicCost = settings.iHeuristicWeight * pathHeuristic(x, y, dest_x, dest_y);
				// total cost will be set when the parent is set.

				child_node->m_bOnStack = true;

				if (pathValid_source(child_node, settings.pGroup , settings.iFlags))
				{
					open_list.push_back(child_node);
					child_node->m_eFAStarListType = FASTARLIST_OPEN;
				}
				else
				{
					child_node->m_eFAStarListType = FASTARLIST_CLOSED; // This node is a dead end.
				}
			}
			else
			{
				// can't get to the plot from here.
				child_node = NULL;
			}
		}
		else
		{
			if (!pathValid_join(parent_node, child_node, settings.pGroup , settings.iFlags))
				child_node = NULL;
		}

		if (child_node == NULL)
			continue;

		FAssert(child_node->m_iX == x && child_node->m_iY == y);

		if (x == dest_x && y == dest_y)
			end_node = child_node; // We've found our destination - but we still need to finish our calculations

		if (parent_node->m_iKnownCost < child_node->m_iKnownCost)
		{
// RI FIX 9 BEGIN
            const __int64 candidate = (__int64)parent_node->m_iKnownCost +
                pathCost(parent_node, child_node, 666, &settings, 0);
            if (candidate <= 0 || candidate > MAX_INT)
            {
                Reset();
                return false;
            }
            int iNewCost = (int)candidate;
// RI FIX 9 END
			FAssert(iNewCost > 0);

			if (iNewCost < child_node->m_iKnownCost)
			{
// RI FIX 9 BEGIN
                // Validate counts and old-parent membership BEFORE mutating the tree.
                const int capacity = sizeof(parent_node->m_apChildren) / sizeof(parent_node->m_apChildren[0]);
                FAStarNode* oldParent = child_node->m_pParent;
                if (parent_node->m_iNumChildren < 0 || parent_node->m_iNumChildren >= capacity ||
                    (oldParent && (oldParent->m_iNumChildren <= 0 || oldParent->m_iNumChildren > capacity)))
                {
                    Reset();
                    return false;
                }
                if (oldParent)
                {
                    int matches = 0;
                    for (int k = 0; k < oldParent->m_iNumChildren; ++k)
                        if (oldParent->m_apChildren[k] == child_node) ++matches;
                    if (matches != 1) { Reset(); return false; }
                }
// RI FIX 9 END
				int cost_delta = iNewCost - child_node->m_iKnownCost; // new minus old. Negative value.

				child_node->m_iKnownCost = iNewCost;
				child_node->m_iTotalCost = child_node->m_iKnownCost + child_node->m_iHeuristicCost;

				// remove child from the list of the previous parent.
				if (child_node->m_pParent)
				{
					FAssert(!bNewNode);
					FAStarNode* x_parent = child_node->m_pParent;
					// x_parent just lost one of its children. We have to break the news to them.
					// this would easier if we had stl instead of bog arrays.
					int temp = x_parent->m_iNumChildren;
					for (int j = 0; j < x_parent->m_iNumChildren; j++)
					{
						if (x_parent->m_apChildren[j] == child_node)
						{
							// found it.
							for (j++ ; j < x_parent->m_iNumChildren; j++)
								x_parent->m_apChildren[j-1] = x_parent->m_apChildren[j];
							x_parent->m_apChildren[j-1] = 0; // not necessary, but easy enough to keep things neat.

							x_parent->m_iNumChildren--;
						}
					}
					FAssert(x_parent->m_iNumChildren == temp -1);
					// recalculate movement points
					pathAdd(parent_node, child_node, ASNC_PARENTADD_UP, &settings, 0);
				}

				// add child to the list of the new parent
				FAssert(parent_node->m_iNumChildren < NUM_DIRECTION_TYPES);
				parent_node->m_apChildren[parent_node->m_iNumChildren] = child_node;
				parent_node->m_iNumChildren++;
				child_node->m_pParent = parent_node;

				// update the new (reduced) costs for all the grandchildren.
				FAssert(child_node->m_iNumChildren == 0 || !bNewNode);
// RI FIX 9 BEGIN
                if (!ForwardPropagate(child_node, cost_delta))
                {
                    Reset(); // clears end_node too, so GeneratePath cannot report success
                    return false;
                }
// RI FIX 9 END

				FAssert(child_node->m_iKnownCost > parent_node->m_iKnownCost);
			}
		}
		// else parent has higher cost. So there must already be a faster route to the child.
	}
	return true;
}

// RI FIX 9 BEGIN - bounded intrusive DFS; no recursion or extra heap allocation.
bool KmodPathFinder::ForwardPropagate(FAStarNode* head, int cost_delta)
{
    // The old code recomputed each edge's delta from pathCost anyway.
    // m_pStack is unused by this Kmod finder, so it can hold the pending list.
    (void)cost_delta;
    const size_t limit = (size_t)map_width * map_height;
    size_t visited = 0;
    head->m_pStack = NULL;
    FAStarNode* pending = head;
    while (pending)
    {
        if (++visited > limit) return false; // cyclic/duplicate tree cannot run forever
        FAStarNode* node = pending;
        pending = node->m_pStack;
        node->m_pStack = NULL;
        const int capacity = sizeof(node->m_apChildren) / sizeof(node->m_apChildren[0]);
        if (node->m_iNumChildren < 0 || node->m_iNumChildren > capacity) return false;
        // Reverse push preserves the original depth-first sibling order.
        // Movement/cost recalculation is performed on pop, after the parent.
        if (node != head)
        {
            FAStarNode* parent = node->m_pParent;
            if (!parent) return false;
            const int oldMoves = node->m_iData1;
            const int oldTurns = node->m_iData2;
            const int oldCost = node->m_iKnownCost;
            pathAdd(parent, node, ASNC_PARENTADD_UP, &settings, 0);
            const __int64 known = (__int64)parent->m_iKnownCost + pathCost(parent, node, 666, &settings, 0);
            const __int64 total = known + node->m_iHeuristicCost;
            if (known <= 0 || known > MAX_INT || total < 0 || total > MAX_INT) return false;
            node->m_iKnownCost = (int)known;
            node->m_iTotalCost = (int)total;
            if (oldCost == node->m_iKnownCost && oldMoves == node->m_iData1 && oldTurns == node->m_iData2)
                continue; // retain original pruning rule
        }
        for (int i = node->m_iNumChildren - 1; i >= 0; --i)
        {
            FAStarNode* child = node->m_apChildren[i];
            if (!child || child == node || child->m_pParent != node) return false;
            child->m_pStack = pending;
            pending = child;
        }
    }
    return true;
}
// RI FIX 9 END
// RI FIX 2 BEGIN - retain storage, never retain UI search state between refreshes.
void KmodPathFinder::Clear()
{
    Reset();
    settings = CvPathSettings();
}

KmodPathFinder& KmodPathFinder::ColoredPlotFinder()
{
    static KmodPathFinder finder; // DLL-owned; CvGame's exported layout is unchanged
    riColoredPlotFinder = &finder;
    return finder;
}

void KmodPathFinder::ClearColoredPlotFinder()
{
    if (riColoredPlotFinder)
        riColoredPlotFinder->Clear();
}
// RI FIX 2 END