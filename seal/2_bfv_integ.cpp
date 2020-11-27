// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.


#include "examples.h"
#include <string>
#include <iostream>
#include <istream>
#include <streambuf>


using namespace std;
using namespace seal;

void example_bfv_integ(int input_x=1, int input_y=2)
{
  print_example_banner("Example: Seal Integ");


  // encryption_parameters 
  EncryptionParameters parms(scheme_type::BFV);
  size_t poly_modulus_degree = 4096;
  parms.set_poly_modulus_degree(poly_modulus_degree);
  parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
  parms.set_plain_modulus(1024);


  // key_generator
  auto context = SEALContext::Create(parms);
  // SEALContext context(parms);
  KeyGenerator keygen(context);
  SecretKey secret_key = keygen.secret_key();
  PublicKey public_key = keygen.public_key();

  //encryption_tools
  Encryptor encryptor(context, public_key);
  Evaluator evaluator(context);
  Decryptor decryptor(context, secret_key);

  //testing save/load of ciphertext
  // max value is 2*16 = 32 : It wraps around at 32
  int x = input_x;

  int y = input_y;

  
  std::stringstream ss_input_x(""),ss_input_y("");

  ss_input_x << std::hex << x;
  ss_input_y << std::hex << y;

  cout<<" The value of x " << x  << " and Ox " << ss_input_x.str() << endl;
  cout<<" The value of y " << y  << " and Ox " << ss_input_y.str() << endl;

  //pass value in hex 
  Plaintext x_plain(ss_input_x.str()), y_plain(ss_input_y.str());

  Ciphertext x_cipher, y_cipher;

  encryptor.encrypt(x_plain,x_cipher);
  encryptor.encrypt(y_plain, y_cipher);

  Ciphertext xy_sum_cipher, xy_prod_cipher;

  print_line(__LINE__);
  cout << "Evaluator: add_inplace : Begin " << endl;
  cout << "         + initialize xy_sum_cipher to x_cipher" << endl;
  xy_sum_cipher = x_cipher;
  cout << "         + add_inplace y_cipher to xy_sum_cipher " << endl;
  evaluator.add_inplace(xy_sum_cipher,y_cipher);
  cout << "Evaluator: add_inplace : End " << endl;

  Ciphertext x_sq_cipher, y_sq_cipher, xy_sumsq_cipher;
  evaluator.square(x_cipher,x_sq_cipher);
  xy_sumsq_cipher = x_sq_cipher;
  evaluator.square(y_cipher,y_sq_cipher);
  evaluator.add_inplace(xy_sumsq_cipher,y_sq_cipher);

  Plaintext xy_sum_plain,xy_sumsq_plain;


  print_line(__LINE__);
  decryptor.decrypt(xy_sum_cipher,xy_sum_plain);
  cout<<" The value of sum " << xy_sum_plain.to_string() << " and Ox " << xy_sum_plain.to_string() << endl;
  decryptor.decrypt(xy_sumsq_cipher,xy_sumsq_plain);
  cout<<" The value of prod " << xy_sumsq_plain.to_string() << " and Ox " << xy_sumsq_plain.to_string() << endl;


//   Ciphertext x_encrypted;
//   Plaintext x_decrypted;
//   Ciphertext y_encrypted;
//   Ciphertext encrypted_result;
//   std::stringstream ss;
//   std::stringstream st;
//   Ciphertext yx_encrypted;
//   Plaintext yx_decrypted;
//   Ciphertext yx_squared;
//   Plaintext yx_sq_decrypted;
//   int xx;

//   //encrypt & decrypt - check
//   encryptor.encrypt(x_plain, x_encrypted);
//   decryptor.decrypt(x_encrypted, x_decrypted);
  

//   cout << " Decryption of x_encrypted  " << endl;
//   cout << " is " << x_decrypted.to_string() << endl; // " and integer value " << xx << endl;

//   // save, load, decrypt as string  - check
//   x_encrypted.save(ss);  
//   yx_encrypted.load(context, ss);
//   decryptor.decrypt(yx_encrypted, yx_decrypted);
  
//   cout << " Save x_encrypted and load yx_encrypted as string" << endl;
//   cout << " yx decrypted is " << yx_decrypted.to_string() << endl;

// /* Array conversion has multipe issues - don't know size, cannot send size, etc.
//   // // save, load, decrypt as byte array - check
//   // auto length = x_encrypted.size();
//   // //std::vector<SEAL_BYTE> bytes(length);
//   // // std::unique_ptr<SEAL_BYTE[]> bytes(new SEAL_BYTE[length]);
//   // auto bytes = make_unique<SEAL_BYTE[]>(length);
//   // x_encrypted.save(bytes.get(),length);
//   // yx_encrypted.load(context, bytes.get(),length); //length has to be passed!
//   // decryptor.decrypt(yx_encrypted, yx_decrypted);
  
//   // cout << " Save x_encrypted and load yx_encrypted as bytes" << endl;
//   // cout << " yx decrypted is " << yx_decrypted.to_string() << endl;
// */

//   //finding square of loaded ciphertext yx_encrypted
//   evaluator.square(yx_encrypted, yx_squared);

//   // Ciphertext assign;
//   // assign = yx_squared;

//   decryptor.decrypt(yx_squared, yx_sq_decrypted);
  
//   //hex string to int
//   // https://stackoverflow.com/questions/1070497/c-convert-hex-string-to-signed-integer
//   st << std::hex << yx_sq_decrypted.to_string();
//   st >> xx;
//   cout << " Evaluate square on yx_encrypted to yx_squared " << endl;
//   cout << " Decryption yx_squared 0x" << yx_sq_decrypted.to_string() << endl;
//   cout << " and in integer value " << xx << endl;


//   cout << endl;


}


/*
You can serialize the Ciphertext to a stream with Ciphertext::save(std::ostream &); 
the stream will then contain all of the full Ciphertext data. 
Then do with it whatever you want: 
  use libb64 to base64-encode it, or write your own encoder, 
  or save it to a file and then base64-encode in the future. 
It might make sense in the future for SEAL to contain an automatic option for serializing 
in base64.
 Ref: https://github.com/microsoft/SEAL/issues/6 
*/

