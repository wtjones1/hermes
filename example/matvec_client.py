import math
import zmq
import numpy as np

from sys            import exit
from matvec_rpc     import Matvec, Problem

port = 49200

context = zmq.Context()
client = Matvec.Client(context, 'tcp://localhost:%d'%port, zmq.REQ)

p = Problem([ 0.,-1., 0.,
              1., 0., 0.,
              0., 0., 1.],
            [1.,2.,3.])

b = client.dgemv(p)

print(f"b = {b}")

assert b[0] == -2. and b[1] == 1. and b[2] == 3., "Bad multiply"
