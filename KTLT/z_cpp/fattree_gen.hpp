std::vector<std::vector<int>> fattree_gen(int k) {
    // Ensure k is even
    if (k % 2 != 0) return {};

    // Calculate number of nodes
    int numPods = k;
    int numCoreSwitches = (k / 2) * (k / 2);
    int numAggSwitchesPerPod = k / 2;
    int numEdgeSwitchesPerPod = k / 2;
    int numServersPerEdge = k / 2;
    int numServersPerPod = numEdgeSwitchesPerPod * numServersPerEdge;
    int totalSwitches = numCoreSwitches + numPods * (numAggSwitchesPerPod + numEdgeSwitchesPerPod);
    int totalNodes = totalSwitches + numPods * numServersPerPod;

    // Initialize adjacency list
    std::vector<std::vector<int>> adjList(totalNodes);

    // Connect core switches to aggregation switches
    int coreSwitchId = 0;
    for (int i = 0; i < k / 2; ++i) { // Groups of core switches
        for (int j = 0; j < k / 2; ++j) { // Core switches in each group
            int coreId = coreSwitchId++;
            for (int pod = 0; pod < numPods; ++pod) {
                // Connect to the j-th aggregation switch in each pod
                int aggId = numCoreSwitches + pod * numAggSwitchesPerPod + j;
                adjList[coreId].push_back(aggId);
                adjList[aggId].push_back(coreId);
            }
        }
    }

    // Connect aggregation switches to edge switches within each pod
    for (int pod = 0; pod < numPods; ++pod) {
        int aggBaseId = numCoreSwitches + pod * numAggSwitchesPerPod;
        int edgeBaseId = numCoreSwitches + numPods * numAggSwitchesPerPod + pod * numEdgeSwitchesPerPod;
        for (int i = 0; i < numAggSwitchesPerPod; ++i) {
            int aggId = aggBaseId + i;
            for (int j = 0; j < numEdgeSwitchesPerPod; ++j) {
                int edgeId = edgeBaseId + j;
                adjList[aggId].push_back(edgeId);
                adjList[edgeId].push_back(aggId);
            }
        }
    }

    // Connect edge switches to servers
    int serverId = totalSwitches;
    for (int pod = 0; pod < numPods; ++pod) {
        int edgeBaseId = numCoreSwitches + numPods * numAggSwitchesPerPod + pod * numEdgeSwitchesPerPod;
        for (int i = 0; i < numEdgeSwitchesPerPod; ++i) {
            int edgeId = edgeBaseId + i;
            for (int j = 0; j < numServersPerEdge; ++j) {
                adjList[edgeId].push_back(serverId);
                adjList[serverId].push_back(edgeId);
                serverId++;
            }
        }
    }

    return adjList;
}