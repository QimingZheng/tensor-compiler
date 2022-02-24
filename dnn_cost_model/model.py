from os import environ
import torch
import torch.nn as nn
"""
Computation Features: # Arithmetic operations / # Memory accesses.
Loop Features: Looping range, transformations.
"""


class Model_Recursive_LSTM_v2(nn.Module):
    def __init__(self,
                 computation_feature_dim,
                 loop_feature_dim,
                 embedding_dim,
                 comp_embed_layer_sizes=[128, 256, 256, 256],
                 concat_layer_sizes=[128, 256, 256],
                 regression_layer_sizes=[128, 256],
                 drop_rate=0.1):
        super().__init__()

        comp_embed_layer_sizes = [computation_feature_dim
                                  ] + comp_embed_layer_sizes + [embedding_dim]
        # The computation-embedding-layers embeds the computation feature vectors into embedding features of `embedding_dim` length.
        self.comp_embedding_layers = nn.ModuleList()
        self.comp_embedding_dropouts = nn.ModuleList()
        for i in range(len(comp_embed_layer_sizes) - 1):
            self.comp_embedding_layers.append(
                nn.Linear(comp_embed_layer_sizes[i],
                          comp_embed_layer_sizes[i + 1],
                          bias=True))
            nn.init.xavier_uniform_(self.comp_embedding_layers[i].weight)
            self.comp_embedding_dropouts.append(nn.Dropout(drop_rate))

        loop_embed_layer_sizes = [loop_feature_dim
                                  ] + comp_embed_layer_sizes + [embedding_dim]
        self.loop_embedding_layers = nn.ModuleList()
        self.loop_embedding_dropouts = nn.ModuleList()
        for i in range(len(loop_embed_layer_sizes) - 1):
            self.loop_embedding_layers.append(
                nn.Linear(loop_embed_layer_sizes[i],
                          loop_embed_layer_sizes[i + 1],
                          bias=True))
            nn.init.xavier_uniform_(self.loop_embedding_layers[i].weight)
            self.loop_embedding_dropouts.append(nn.Dropout(drop_rate))

        # The LSTM unit used to represent the computation embeddings
        self.comps_lstm = nn.LSTM(embedding_dim,
                                  embedding_dim,
                                  batch_first=True)
        # The LSTM unit used to represent the looping embeddings
        self.nodes_lstm = nn.LSTM(embedding_dim,
                                  embedding_dim,
                                  batch_first=True)

        # The concatenation layers concat the computation embeddings/loop embeddings (from the two LSTMs) and the input looping-feature vector and transform the concat results with a feed-forward network.
        concat_layer_sizes = [
            embedding_dim * 3  # + loop_feature_dim
        ] + concat_layer_sizes + [embedding_dim]
        self.concat_layers = nn.ModuleList()
        self.concat_dropouts = nn.ModuleList()
        for i in range(len(concat_layer_sizes) - 1):
            self.concat_layers.append(
                nn.Linear(concat_layer_sizes[i],
                          concat_layer_sizes[i + 1],
                          bias=True))
            nn.init.xavier_uniform_(self.concat_layers[i].weight)
            self.concat_dropouts.append(nn.Dropout(drop_rate))

        # The regression layers accepts the output of the concat-layers module, and output the predicted speedups.
        regression_layer_sizes = [embedding_dim
                                  ] + regression_layer_sizes + [embedding_dim]
        self.regression_layers = nn.ModuleList()
        self.regression_dropouts = nn.ModuleList()
        for i in range(len(regression_layer_sizes) - 1):
            self.regression_layers.append(
                nn.Linear(regression_layer_sizes[i],
                          regression_layer_sizes[i + 1],
                          bias=True))
            nn.init.xavier_uniform_(self.regression_layers[i].weight)
            self.regression_dropouts.append(nn.Dropout(drop_rate))
        self.predict = nn.Linear(regression_layer_sizes[-1], 1, bias=True)
        nn.init.xavier_uniform_(self.predict.weight)

        # Some trainable constants.
        self.ELU = nn.ELU()
        self.no_comps_tensor = nn.Parameter(
            nn.init.xavier_uniform_(torch.zeros(1, 1, embedding_dim)))
        self.no_nodes_tensor = nn.Parameter(
            nn.init.xavier_uniform_(torch.zeros(1, 1, embedding_dim)))

    """
    A tree structure LSTM model, Embeds the computation-feature and looping-feature of each ast-node.
    """

    def get_hidden_state(self, node, comps_embeddings, loops_tensor):
        nodes_list = []
        # Gather all the (direct) child-looping nodes' hidden states after go through the LSTM module.
        for n in node['child_list']:
            nodes_list.append(
                self.get_hidden_state(n, comps_embeddings, loops_tensor))

        # When contains direct child-loopings, concat these Loop's hidden state into a sequence of hidden states as the input to the node-LSTM units.
        # Otherwise, use the placeholder empty hidden state.
        if (nodes_list != []):
            nodes_tensor = torch.cat(nodes_list, 1)
            lstm_out, (nodes_h_n, nodes_c_n) = self.nodes_lstm(nodes_tensor)
            nodes_h_n = nodes_h_n.permute(1, 0, 2)
        else:
            nodes_h_n = self.no_nodes_tensor

        if (node['has_comps']):
            selected_comps_tensor = torch.index_select(
                comps_embeddings, 1, node['computations_indices'])
            lstm_out, (comps_h_n,
                       comps_c_n) = self.comps_lstm(selected_comps_tensor)
            comps_h_n = comps_h_n.permute(1, 0, 2)
        else:
            comps_h_n = self.no_comps_tensor

        # Get the looping-feature of current loop.
        selected_loop_tensor = torch.index_select(loops_tensor, 1,
                                                  node['loop_index'])
        for i in range(len(self.loop_embedding_layers)):
            selected_loop_tensor = self.loop_embedding_layers[i](
                selected_loop_tensor)
            selected_loop_tensor = self.loop_embedding_dropouts[i](
                self.ELU(selected_loop_tensor))
        # Concat the above embeddings and current-loop's looping feature.
        x = torch.cat((nodes_h_n, comps_h_n, selected_loop_tensor), 2)
        for i in range(len(self.concat_layers)):
            x = self.concat_layers[i](x)
            x = self.concat_dropouts[i](self.ELU(x))
        return x

    def forward(self, tree_tensors):
        # All the computation-feature-tensors and looping-feature-tensors in one batch should share the same ast structure.
        tree, comps_tensor, loops_tensor = tree_tensors
        # computation embbedding layer
        x = comps_tensor
        for i in range(len(self.comp_embedding_layers)):
            x = self.comp_embedding_layers[i](x)
            x = self.comp_embedding_dropouts[i](self.ELU(x))
        comps_embeddings = x
        # recursive loop embbeding layer
        prog_embedding = self.get_hidden_state(tree, comps_embeddings,
                                               loops_tensor)
        # regression layer
        x = prog_embedding
        for i in range(len(self.regression_layers)):
            x = self.regression_layers[i](x)
            x = self.regression_dropouts[i](self.ELU(x))
        out = self.predict(x)

        return self.ELU(out[:, 0, 0])
