Some commands im using:

opt \
  -load-pass-plugin=/content/build/MyPass/libPass1.so \
  -passes=ILPPass \
  /content/O1.ll \
  -S -o /content/O1-ILP.ll

  