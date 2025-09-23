#![no_std]
#![no_main]
use core::panic::PanicInfo;

unsafe extern "C" {
    safe fn printf(format: *const u8, ...);
}

#[no_mangle]
pub extern "C" fn test_rust() {
    let mut test = [0u8; 3];
    test[0] = b'H';
    test[1] = b'W';
    printf(&test as *const u8);
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
