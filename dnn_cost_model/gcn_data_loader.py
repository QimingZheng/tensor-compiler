from concurrent.futures import process
import dgl
from dgl.data import DGLDataset
import torch
import os
import json
import numpy as np
import re


class SyntheticData(DGLDataset):
    def __init__(self):
        super().__init__(name='synthetic-data')
        self.trees = [self.process() for i in range(10)]

    def process(self):
        node_num = 5
        node_features = torch.ones((5, 16))
        node_labels = torch.zeros((5), dtype=np.long)
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


class GCNData(DGLDataset):
    def __init__(self, filename):
        super().__init__(name='dgn-data')
        self.trees = []
        with open("train.json") as F:
            line = F.readline()
            while line:
                line = re.sub(",[\t\r\n]*}", "}", line)
                line = re.sub(",[\t\r\n]*\]", "]", line)
                sample = json.loads(line)
                self.trees.append(self.preprocess(sample))
                line = F.readline()
        # self.trees = [self.process() for i in range(10)]

    def process(self):
        return

    def preprocess(self, sample):
        node_num = len(sample['nodes'])
        node_features = []
        for node in sample['nodes']:
            if len(node['feature']) == 1:
                node_features.append(node['feature'] * 2)
            else:
                node_features.append(node['feature'])
        node_features = torch.tensor(node_features)
        node_labels = torch.tensor([sample['cost']] * node_num,
                                   dtype=torch.float32)
        edge_num = len(sample['edges'])
        edge_features = torch.zeros((edge_num, 1))

        edges_src = []
        edges_dst = []
        for edge in sample['edges']:
            edges_src.append(edge[1])
            edges_dst.append(edge[0])

        edges_src = torch.tensor(edges_src, dtype=np.long)
        edges_dst = torch.tensor(edges_dst, dtype=np.long)

        graph = dgl.graph((edges_src, edges_dst), num_nodes=node_num)
        graph.ndata['feat'] = node_features
        graph.ndata['label'] = node_labels
        graph.edata['weight'] = edge_features
        return graph

    def __getitem__(self, i):
        return self.trees[i]

    def __len__(self):
        return len(self.trees)
