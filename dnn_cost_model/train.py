from os import environ
import numpy as np
import torch
from torch import optim
import time
import math
import copy

from data_loader import *
from model import *

environ["train_device"] = "cuda:0"
environ["store_device"] = "cuda:0"
environ["dataset_file"] = "./data/train.txt"

train_device = torch.device(environ.get('train_device'))
store_device = torch.device(environ.get('store_device'))
dataset_file = environ.get('dataset_file')
test_dataset_file = environ.get('test_dataset_file')
benchmark_dataset_file = environ.get('benchmark_dataset_file')


def mape_criterion(inputs, targets):
    eps = 1e-5
    return 100 * torch.mean(torch.abs(targets - inputs) / (targets + eps))


def abs_criterion(inputs, targets):
    return torch.mean(torch.abs(targets - inputs))


def sqr_criterion(inputs, targets):
    return torch.mean(torch.square(targets - inputs))


def train_model(model,
                criterion,
                optimizer,
                max_lr,
                dataloader,
                num_epochs=100,
                log_every=5,
                logFile='log.txt'):
    since = time.time()
    losses = []
    train_loss = 0
    best_loss = math.inf
    best_model = None
    dataloader_size = {'train': 0, 'val': 0}
    for _, label in dataloader['train']:
        dataloader_size['train'] += label.shape[0]
    for _, label in dataloader['val']:
        dataloader_size['val'] += label.shape[0]

    model = model.to(train_device)
    scheduler = torch.optim.lr_scheduler.OneCycleLR(optimizer,
                                                    max_lr=max_lr,
                                                    steps_per_epoch=len(
                                                        dataloader["train"]),
                                                    epochs=num_epochs)
    for epoch in range(num_epochs):
        epoch_start = time.time()
        # Each epoch has a training and validation phase
        for phase in ['train', 'val']:
            if phase == 'train':
                model.train()
            else:
                model.eval()
            running_loss = 0.0
            # Iterate over data.
            for inputs, labels in dataloader[phase]:
                original_device = labels.device
                inputs = (inputs[0], inputs[1].to(train_device),
                          inputs[2].to(train_device))
                labels = labels.to(train_device)
                # zero the parameter gradients
                optimizer.zero_grad()
                # forward
                # track history if only in train
                with torch.set_grad_enabled(phase == 'train'):
                    outputs = model(inputs)
                    assert outputs.shape == labels.shape
                    # print((outputs, labels))
                    loss = criterion(outputs, labels)
                    # backward + optimize only if in training phase
                    if phase == 'train':
                        loss.backward()
                        optimizer.step()
                # statistics
                running_loss += loss.item() * labels.shape[0]
                inputs = (inputs[0], inputs[1].to(original_device),
                          inputs[2].to(original_device))
                labels = labels.to(original_device)
                epoch_end = time.time()
                # running_corrects += torch.sum((outputs.data - labels.data) < e)/inputs.shape[0]
            epoch_loss = running_loss / dataloader_size[phase]
            if phase == 'val':
                losses.append((train_loss, epoch_loss))
                if (epoch_loss <= best_loss):
                    best_loss = epoch_loss
                    best_model = copy.deepcopy(model)
                print(
                    'Epoch {}/{}:  train Loss: {:.4f}   val Loss: {:.4f}   time: {:.2f}s   best: {:.4f}'
                    .format(epoch + 1, num_epochs, train_loss, epoch_loss,
                            epoch_end - epoch_start, best_loss))
                if (epoch % log_every == 0):
                    with open(logFile, "a+") as f:
                        f.write(
                            'Epoch {}/{}:  train Loss: {:.4f}   val Loss: {:.4f}   time: {:.2f}s   best: {:.4f} \n'
                            .format(epoch + 1, num_epochs, train_loss,
                                    epoch_loss, epoch_end - epoch_start,
                                    best_loss))
            else:
                train_loss = epoch_loss
                scheduler.step()
    time_elapsed = time.time() - since
    print(
        'Training complete in {:.0f}m {:.0f}s   best validation loss: {:.4f}'.
        format(time_elapsed // 60, time_elapsed % 60, best_loss))
    with open(logFile, "a+") as f:
        f.write(
            '-----> Training complete in {:.0f}m {:.0f}s   best validation loss: {:.4f}\n '
            .format(time_elapsed // 60, time_elapsed % 60, best_loss))

    return losses, best_model


def transform(tree):
    if 'computations_indices' in tree:
        tree["computations_indices"] = torch.tensor(
            tree["computations_indices"]).to(train_device)
    if 'loop_index' in tree:
        tree["loop_index"] = torch.tensor(tree["loop_index"]).to(train_device)
    for child in tree['child_list']:
        transform(child)


def prepare_dataset(file_name):
    train_data = read_data(file_name)
    [transform(data) for data in train_data]
    print("Sample number: ", len(train_data))
    train_bl = [
        ((data, torch.FloatTensor([data["computation_feature_tensors"]
                                   ]).to(train_device),
          torch.FloatTensor([data["loop_feature_tensors"]]).to(train_device)),
         torch.FloatTensor([math.log(data["label"])]).to(train_device))
        for data in train_data
    ]
    return train_bl


if __name__ == "__main__":
    computation_feature_dim = 2
    loop_feature_dim = 3
    embedding_dim = 512

    train_bl = prepare_dataset(sys.argv[1])
    val_bl = prepare_dataset(sys.argv[2])

    model = Model_Recursive_LSTM_v2(computation_feature_dim, loop_feature_dim,
                                    embedding_dim)
    train_model(model, sqr_criterion, optim.AdamW(model.parameters()), 0.001, {
        'train': train_bl,
        'val': val_bl
    }, 1000)
