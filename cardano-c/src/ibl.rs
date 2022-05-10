
use cardano::hdwallet::{self, XPrv, XPRV_SIZE};
use std::{ffi, slice, ptr};
use std::os::raw::{c_char};
use cardano::bip::bip39::{self, Mnemonics, MnemonicString, dictionary};
use std::println;

#[no_mangle]
pub extern "C"
fn create_rootkey( mnemonics: *const c_char
                 , password:  *const c_char )
-> *mut c_char
{
    let mnemonics     = unsafe {ffi::CStr::from_ptr(mnemonics)};
    let mnemonics_str = mnemonics.to_str().unwrap();
    let mnemonics     = MnemonicString::new(&dictionary::ENGLISH, mnemonics_str.to_string()).unwrap();

    let password      = unsafe {ffi::CStr::from_ptr(password)};
    let password_str  = password.to_str().unwrap();
    let password      = password_str.as_bytes();
    let seed = bip39::Seed::from_mnemonic_string(&mnemonics, &password);

    let xprv = XPrv::generate_from_bip39(&seed);

    println!("{}", xprv.to_string());
    ffi::CString::new(xprv.to_string()).unwrap().into_raw()
}
