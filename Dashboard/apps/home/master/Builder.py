import struct

import os
import shutil
import pyzipper
from werkzeug.utils import secure_filename






def patch_exe(file_path, patches, output_path, wchar_length=256):
    """
    Applies UTF-16LE string patches to a binary file.

    - Searches for *short* UTF-16LE strings (no padding)
    - Replaces with padding up to wchar_length
    """
    with open(file_path, 'rb') as f:
        data = f.read()

    patch_list = []

    for i, patch in enumerate(patches):
        old_str = patch['old_value']
        new_str = patch['new_value']

        # Encode search string WITHOUT padding
        old_bytes = old_str.encode('utf-16le')
        new_bytes = new_str.encode('utf-16le')

        if len(new_bytes) > wchar_length * 2:
            raise ValueError(f"New string '{new_str}' too long for {wchar_length} wchar_t buffer")

        # Replacement bytes: padded to fill buffer
        new_bytes_padded = new_bytes.ljust(wchar_length * 2, b'\x00')

        offset = data.find(old_bytes)
        if offset == -1:
            raise ValueError(f"UTF-16LE string '{old_str}' not found")

        print(f"[Patch {i+1}] '{old_str}' found at offset {offset:#x}")
        patch_list.append((offset, len(old_bytes), new_bytes_padded))

    # Apply patches
    new_data = bytearray(data)
    for offset, old_len, new_bytes_padded in patch_list:
        # Overwrite up to wchar_length bytes starting at offset
        new_data[offset:offset + wchar_length * 2] = new_bytes_padded

    with open(output_path, 'wb') as f:
        f.write(new_data)

    print(f"\n✅ Applied {len(patch_list)} UTF-16LE string patches to '{output_path}'")
    for i, (offset, old_len, new_bytes_padded) in enumerate(patch_list):
        new_str = new_bytes_padded.decode('utf-16le').rstrip('\x00')
        print(f"  Patch {i+1}: replacement → '{new_str}' at {offset:#x}")
  
def zip_and_clean(outpute_exe, client_name, zip_password,zip_file_path=None):
    output_dir = os.path.dirname(outpute_exe)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    
    client_name = secure_filename(client_name)
    
    if os.path.exists(zip_file_path):
        os.remove(zip_file_path)

    # Collect files to zip
    files_to_zip = []
    for root, dirs, files in os.walk(output_dir):
        for file in files:
            file_path = os.path.join(root, file)
            if file_path != zip_file_path:
                arcname = os.path.relpath(file_path, output_dir)
                files_to_zip.append((file_path, arcname))


    

    # Create password-protected zip
    with pyzipper.AESZipFile(zip_file_path, 'w', compression=pyzipper.ZIP_LZMA) as zf:
        zf.setpassword(zip_password.encode('utf-8'))
        zf.setencryption(pyzipper.WZ_AES, nbits=256)
        for file_path, arcname in files_to_zip:
            zf.write(file_path, arcname)

    print(f"Output Dire: {output_dir}")

    # Clean the folder (optional)
    for f, _ in files_to_zip:
        os.remove(f)
    print("Cleaned up original files.")




















