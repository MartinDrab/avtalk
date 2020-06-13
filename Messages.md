# Messages

## Message header

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 4    | Header size |
| 4 | 4 | Flags |
| 8 | 32   | Header signature |
| 40 | 32 | Header encryption |
| 72 | 4    | Message type |
| 76 | 4 | Data size |
| 80 | 8 | Packet counter |
| 88 | 16 | Sender GUID |
| 104 | 16 | Recipient GUID |

* *Header Signature* must be done with sender's public key.
* *Header encryption* must be done with recipient's (or server's) public key.
* *Packet counter* protects against replay attack. Each packet must contain unique value of this counter coupled with the *Source GUID* one.
* *Source GUID* and *Destination GUID* identify a server, user, session or an AV stream.
* *Flags* indicate whether the data are signed (**1**), data are encrypted (**2**), header is signed (**4**), header is encrypted (**8**).

## Get Server Information

### Input

* *Header signature* = **zeroed**
* *Header encryption* = may be encrypted with server's key
* *Message type* = 1
* *Sender GUID* = `GUID_NULL`
* *Recipient GUID* = `GUID_NULL`
* No extra data

### Output

* *Header Signature* = **signed by the server**
* *Message type* = 1
* *Sender GUID* = server GUID
* *Flags* = 1
* Server public key (64 bytes)
* Server name (256 characters)
* Visible users (GUID, name) (128 bytes)
* Visible sessions (GUID, name, user count) (128 bytes)

## New user

### Input

* *Header signature* = **Signed by the user**
* *Header encryption* = **Encrypted for the server**
* *Message type* = 2
* *Sender GUID* = `GUID_NULL`
* *Recipient GUID* = server GUID
* Data signed by the user, encrypted for the server
* user public key
* user visible server-wide
* user expiration date
* possibly other user attributes

### Output

* *Header signature* = **signed by the server**
* *Header encryption* = **encrypted for the user**
* *Message type* = 2
* *Sender GUID* = server GUID
* *Recipient GUID* = user GUID provided by the server
* Challenge for testing user's ability to sign

## Remove user

### Input

* *Header signature* = **Signed by the user**
* *Header encryption* = encrypted for the server
* *Message type* = 3
* *Sender GUID* = user GUID
* *Recipient GUID* = server GUID

### Output

None
