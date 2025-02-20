# STHERM Monitoring/protobuffer



To integrate updated Protocol Buffer definitions into the monitoring system:

1. **Obtain `protoc`:** Download the appropriate `protoc` compiler binaries for your operating system (Windows or Linux) from the official Protocol Buffers releases page: [https://github.com/protocolbuffers/protobuf/releases](https://github.com/protocolbuffers/protobuf/releases).


2. **Code Generation Commands:**

   * **Windows:** Open a command prompt or PowerShell in the directory containing `protoc.exe` and run:

     ```bash
     .\protoc.exe --cpp_out=.\Backend\Proto .\streamdata.proto
     ```

     The `--cpp_out=.\Backend\Proto` option directs `protoc` to output the generated files to the `.\Backend\Proto` directory (relative to the current directory). Ensure this directory exists.

   * **Linux:** Open a terminal, navigate to the directory containing the `protoc` executable, and run:

     ```bash
     ./protoc --cpp_out=./Backend/Proto ./streamdata.proto
     ```

     Remember to replace `streamdata.proto` with the actual name of your `.proto` file.  Also, ensure that `protoc` has execute permissions: `chmod +x protoc`.


3. **Update `.proto` and Generate Code:** When adding new fields to the monitoring system, you'll receive an updated `.proto` file.  Use this file to generate the corresponding C++ classes (`*.pb.h` and `*.pb.cc`).  These generated files need to be placed in the `Backend/Proto` directory, replacing the older versions.  Afterward, update the `ProtoDataManager` class to handle the new fields.
