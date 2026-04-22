from __future__ import print_function
import struct
import sys

def xdr_pack_string(buffer, s):
    """Pack a variable-length string in XDR format.
    
    Args:
        buffer: bytearray to append packed data to
        s: string to pack
    """
    if sys.version_info[0] < 3:
        s_bytes = s if isinstance(s, bytes) else s.encode('utf-8')
    else:
        s_bytes = s.encode('utf-8') if isinstance(s, str) else s
    
    length = len(s_bytes)
    # Pack the length as unsigned int
    buffer.extend(struct.pack('>I', length))
    # Pack the string data
    buffer.extend(s_bytes)
    # Add padding to 4-byte boundary
    padding = (4 - (length % 4)) % 4
    buffer.extend(b'\x00' * padding)

def xdr_unpack_string(data, position):
    """Unpack a variable-length string in XDR format.
    
    Args:
        data: bytes object containing packed data
        position: current position in data
        
    Returns:
        tuple of (unpacked_string, new_position)
    """
    # Unpack the length
    length = struct.unpack_from('>I', data, position)[0]
    position += 4
    # Unpack the string data
    s_bytes = data[position:position + length]
    position += length
    # Skip padding to 4-byte boundary
    padding = (4 - (length % 4)) % 4
    position += padding
    
    if sys.version_info[0] < 3:
        return (s_bytes, position)
    else:
        return (s_bytes.decode('utf-8'), position)
