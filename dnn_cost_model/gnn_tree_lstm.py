from collections import namedtuple
import dgl
from dgl.data.tree import SSTDataset
import torch as th
import torch.nn as nn
import dgl.function as fn
from torch.utils.data import DataLoader
import torch.nn.functional as F

from gcn_data_loader import SyntheticData

SSTBatch = namedtuple('SSTBatch', ['graph', 'label'])

trainset = SyntheticData()
tiny_set = trainset.trees

graph = dgl.batch(tiny_set)


class TreeLSTMCell(nn.Module):
    def __init__(self, feat_size, hidden_size):
        super(TreeLSTMCell, self).__init__()
        self.b_feat = nn.Parameter(th.zeros(1, feat_size))
        self.feat_layer = nn.Linear(feat_size, hidden_size, bias=False)
        self.n_feat_layer = nn.Linear(hidden_size, hidden_size, bias=True)

    def message_func(self, edges):
        return {'n_feat': edges.src['n_feat']}

    def reduce_func(self, nodes):
        feat = th.sum(self.n_feat_layer(nodes.mailbox['n_feat']), 1)
        return {'n_feat': feat}

    def apply_node_func(self, nodes):
        feat = nodes.data['feat'] + self.b_feat
        feat = self.feat_layer(feat)
        return {'n_feat': feat}


class TreeLSTM(nn.Module):
    def __init__(self, feature_size, hidden_size, dropout):
        super(TreeLSTM, self).__init__()
        self.feature_size = feature_size
        self.dropout = nn.Dropout(dropout)
        self.linear = nn.Linear(hidden_size, 1)
        self.cell = TreeLSTMCell(feature_size, hidden_size)

    def forward(self, batch):
        g = batch.graph
        # propagate
        dgl.prop_nodes_topo(g,
                            message_func=self.cell.message_func,
                            reduce_func=self.cell.reduce_func,
                            apply_node_func=self.cell.apply_node_func)
        # compute logits
        h = self.dropout(g.ndata.pop('n_feat'))
        logits = self.linear(h)
        return logits


device = th.device('cpu')
# hyper parameters
feat_size = 16
hidden_size = 128
dropout = 0.5
lr = 0.05
weight_decay = 1e-4
epochs = 10

# create the model
model = TreeLSTM(feat_size, hidden_size, dropout)
print(model)

# create the optimizer
optimizer = th.optim.Adagrad(model.parameters(),
                             lr=lr,
                             weight_decay=weight_decay)


def batcher(dev):
    def batcher_dev(batch):
        batch_trees = dgl.batch(batch)
        return SSTBatch(graph=batch_trees,
                        label=batch_trees.ndata['label'].to(device))

    return batcher_dev


train_loader = DataLoader(dataset=tiny_set,
                          batch_size=5,
                          collate_fn=batcher(device),
                          shuffle=False,
                          num_workers=0)

# training loop
for epoch in range(epochs):
    for step, batch in enumerate(train_loader):
        g = batch.graph
        n = g.number_of_nodes()
        logits = model(batch)
        logp = F.log_softmax(logits, 1)
        loss = F.nll_loss(logp, batch.label, reduction='sum')
        optimizer.zero_grad()
        loss.backward()
        optimizer.step()
        pred = th.argmax(logits, 1)
        acc = float(th.sum(th.eq(batch.label, pred))) / len(batch.label)
        print("Epoch {:05d} | Step {:05d} | Loss {:.4f} | Acc {:.4f} |".format(
            epoch, step, loss.item(), acc))
