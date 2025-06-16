

### Chapter 04: Request-Response Protocol  
**Implementing a Simple Binary Protocol for Client-Server Communication**

#### Key Concepts:
1. **Server Loop**  
   - Handles multiple requests sequentially in a single connection using `accept()` and `one_request()`.
   - Simplified model (no concurrency): One client served at a time.

2. **Binary Protocol Design**  
   - **Message Format**: `[4-byte length (little-endian)] + [payload]`  
   - Fixed-length header indicates variable payload size (max 4KB).
   - Avoids complexity of text-based protocols (like HTTP/Redis) and delimiter parsing.

3. **Robust Socket I/O**  
   - **Critical Insight**: `read()`/`write()` may return partial data under normal conditions.  
   - **Helper Functions**:  
     - `read_full()`: Reads exactly `n` bytes (retries on partial reads).  
     - `write_all()`: Writes entire buffer (retries on partial writes).  
   - Handles edge cases: EOF, errors, and interrupted syscalls.

4. **Protocol Implementation**  
   - **Server**: Uses `one_request()` to parse headers, validate length, read payload, and echo responses.  
   - **Client**: `query()` sends requests and reads replies using the same protocol.  
   - Testing: Sequential requests (`hello1`, `hello2`) validate the pipeline.

5. **TCP vs. Disk I/O**  
   - **Push-based (TCP)**: Data arrives asynchronously → partial reads are normal.  
   - **Pull-based (Disk)**: File size known → partial reads imply EOF/error.  

6. **Protocol Tradeoffs**  
   - **Binary > Text**: Simpler parsing than HTTP/JSON (no string operations/delimiter escapes).  
   - **Length-Prefixing > Delimiters**: Explicit sizing avoids delimiter collision issues.  

#### Source Code:
- Server: [`04_server.cpp`](https://build-your-own.org/redis/04_server.cpp)  
- Client: [`04_client.cpp`](https://build-your-own.org/redis/04_client.cpp)  

> **Next Step**: Event loops for concurrent connections (Chapter 05).  

---

