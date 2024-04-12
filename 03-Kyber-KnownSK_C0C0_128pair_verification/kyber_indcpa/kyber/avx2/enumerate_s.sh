for sk_index in {0..9}
do
for guess_z in {1..3329}
do
./test_kyber_indcpa_local $guess_z 0 1 1 $sk_index
done
done
