# Simple Matrix Vector Multiply Example

The Hermes Interface Definition (HID) file defined by matvec_rpc.hid defines data-types and a single interface function.  The HID allows for creating the abstract interfaces in a language agnostic way.  This example defines a simple data structure to contain a flattened MxN matrix (a) and a vector (x) of dimension M.  The single interface call is to perform a simple matrix-vector multiply on the data in the problem structure.  The result is returned as a vector of dimension N.

First, generate the Fortran and Python Remote Procedure Call (RPC) code from the HID.

```
% hermes --fortran matvec_rpc.hid
% hermes --python matvec_rpc.hid
```

These commands will generate, respectively, a Fortran RPC module, matvec_rpc.f90, and then a Python file, matvec_rpc.py.  These contain all of the RPC calls to make the call of the server routines, provide accessors to the data in the data structures defined in the HID, and return data to the client.  The Fortran module is used by Fortran servers and clients, and the Python module is used by Python servers and clients.  Note these can be mixed and matched as will be subsequently used here (Fortran server and a Python client).  Also generated will be matvec_rpc_stubs.\* files that serve as templates for implementing a server in either Fortran or Python.  In these are interface stubs that provide all of the signatures for the interfaces, etc..

Next, compile the provided Fortran server, matvec_server.f90.  Again, this is based on the stub file matvec_rpc_stubs.f90.  **Note**, you must use the same Fortran compiler vendor and version and ZeroMQ used to build Hermes:

```
% gfortran -o matvec_server -I${HERMES_HOME}/include matvec_rpc.f90 \
           matvec_server.f90 -L${HERMES_HOME}/lib -lhermes_fortran \
           -L${ZEROMQ_HOME}/lib -lzmq
```

Now run the server in the background:

```
% ./matvec_server &
```

Finally, run the provided client, matvec_client.py:

```
% python3 matvec_client.py
```

Don't forget the kill your server when finished (left as an exercise to add clean, remote shutdown from client):

```
% fg
% <ctrl-C>
```

You can compare the generated matvec_rpc_stubs.f90 to matvec_server.f90 to see the implementation details of this simple server.  Again, the former file is generated when running Hermes to produce Fortran RPC as a template for implementing your server (stubbed out interfaces).  The matvec_rpc_stubs.py is produced with generating the Python RPC and provides a starting place to implement a Python based server (not used in this example).
