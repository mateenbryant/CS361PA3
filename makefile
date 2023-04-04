all:	tours.c wrappers.c
	gcc tours.c     wrappers.c -pthread -o tours

test:
	echo
	./tours 3 0 0 > out_3_0_0.txt
	diff -s out_3_0_0.txt expected_3_0_0.txt
	echo
	./tours 3 1 0 > out_3_1_0.txt
	diff -s out_3_1_0.txt expected_3_1_0.txt
	echo
	./tours 3 4 0 > out_3_4_0.txt
	./tours 3 1 2 > out_3_1_2.txt
	./tours 3 2 4 > out_3_2_4.txt
	./tours 3 5 2 > out_3_5_2.txt
	./tours 4 4 1 > out_4_4_1.txt
	./tours 5 7 2 > out_5_7_2.txt

clean:
	rm -f tours out*

