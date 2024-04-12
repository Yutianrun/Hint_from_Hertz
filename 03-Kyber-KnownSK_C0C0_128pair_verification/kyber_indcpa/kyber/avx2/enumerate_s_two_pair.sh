# for pair in 5 44 53 56 60 65 69 74 97 106
# do
# for guess_z in {1..3329}
# do
# ./test_kyber_indcpa_local_two_pair $guess_z $pair 1 1 
# done
# done


for pair in 1 
do
for guess_z in {1..3328}
do
./test_kyber_indcpa_local_two_pair $guess_z $pair 1 1 
done
done
