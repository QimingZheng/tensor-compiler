from concurrent.futures import process
import dgl
from dgl.data import DGLDataset
import torch
import os
import json
import numpy as np


class SyntheticData(DGLDataset):
    def __init__(self):
        super().__init__(name='synthetic-data')
        self.trees = [self.process() for i in range(10)]

    def process(self):
        node_num = 5
        node_features = torch.ones((5, 16))
        node_labels = torch.zeros((5), dtype = np.long)
        edge_features = torch.ones((4, 8))
        edges_src = torch.tensor([0, 1, 2, 3], dtype=np.long)
        edges_dst = torch.tensor([1, 2, 3, 4], dtype=np.long)

        graph = dgl.graph((edges_src, edges_dst), num_nodes=node_num)
        graph.ndata['feat'] = node_features
        graph.ndata['label'] = node_labels
        graph.edata['weight'] = edge_features
        return graph

    def __getitem__(self, i):
        return self.trees[i]

    def __len__(self):
        return 10
