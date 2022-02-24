import readline
import sys
import os
import json


def read_data(file_name):
    with open(file_name) as f:
        line = f.readline()
        ret = []
        while line != "":
            ret.append(json.loads(line))
            line = f.readline()
    return ret


if __name__ == "__main__":
    ret = read_data(sys.argv[1])
    print(ret[1]["computation_feature_tensors"])