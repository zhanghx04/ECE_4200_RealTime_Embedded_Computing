all: Lab4.c
	gcc -g -lpthread -lwiringPi -o l Lab4.c libece4220lab3.a
	gcc -g -lpthread -lwiringPi -o b Lab4_button.c libece4220lab3.a
	./b &
	./l

clean :
	$(RM) Lab4
	$(RM) Lab4_button
