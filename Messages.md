# Messages

## Message header

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 32   | Header signature |
| 32 | 4    | Header size |
| 36 | 4    | Message type |
| 40 | 8 | Packet counter |
| 48 | 16 | Sender GUID |
| 64 | 16 | Recipient GUID |
| 80 | 4 | Flags |
| 84 | 4 | Data size |

* *Header Signature* must be done with sender's public key.
* *Packet counter* protects against replay attack. Each packet must contain unique value of this counter coupled with the *Source GUID* one.
* *Source GUID* and *Destination GUID* identify a server, user, session or an AV stream.
* *Flags* indicate whether the data are signed (**1**) or encrypted (**2**).

## Get Server Information

### Input

* *Header signature* = <zeroes>
* *Message type* = 1
* *Sender GUID* = `GUID_NULL
* *Recipient GUID* = `GUID_NULL`
* No extra data

### Output

* *Header Signature* = <signed by the server>
* *Message type* = 1
* *Sender GUID* = <server GUID>
* *Flags* = 1
* Server public key (64 bytes)
* Server name (256 characters)
* Visible users (GUID, name) (128 bytes)
* Visible sessions (GUID, name, user count) (128 bytes)
