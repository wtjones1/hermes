# Simple Matrix Vector Multiply Example

First, generate the Fortran and Python Remote Procedure Call (RPC) code from the Hermes Interface Definition (HID):

```
% hermes --fortran matvec_rpc.hid
% hermes --python matvec_rpc.hid
```

Next, compile the Fortran server (using the same Fortran compiler vendor and version and ZeroMQ used to build Hermes):

```
% gfortran -o matvec_server -I${HERMES_HOME}/include matvec_rpc.f90 \
           matvec_server.f90 -L${HERMES_HOME}/lib -lhermes_fortran \
           -L${ZEROMQ_HOME}/lib -lzmq
```

Now run the server in the background:

```
% ./matvec_server &
```

Finally, run the client:

```
% python3 matvec_client.py
```

Don't forget the kill your server when finished:

```
% fg
% <ctrl-C>
```

You can compare the generated matvec_rpc_stubs.f90 to matvec_server.f90 to see the formatting and implementation details of this simple server.  The former file is generated when running Hermes to produce Fortran RPC as a template for implementing your server (stubbed out interfaces).  The matvec_rpc_stubs.py is produced with generating the Python RPC and provides a starting place to implement a Python based server (not used in this example).
