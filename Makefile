CC = gcc
CFLAGS = -Wall -g -fPIC
LDFLAGS = -lpthread

# Librería del proxy
LIB_PROXY = libproxyclaves.so

# Servidor
SERVER_EXE = servidor-sock

# Clientes
CLIENT1_EXE = cliente1
CLIENT2_EXE = cliente2

all: $(LIB_PROXY) $(SERVER_EXE) $(CLIENT1_EXE) $(CLIENT2_EXE)

# Librería dinámica del proxy
$(LIB_PROXY): proxy-sock.o lines.o
	$(CC) -shared -o $@ $^

# Ejecutable del servidor
$(SERVER_EXE): servidor-sock.o claves.o lines.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Clientes
$(CLIENT1_EXE): app-cliente-1.o $(LIB_PROXY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lproxyclaves $(LDFLAGS)

$(CLIENT2_EXE): app-cliente-2.o $(LIB_PROXY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lproxyclaves $(LDFLAGS)

# Objetos con reglas explícitas
proxy-sock.o: proxy-sock.c
	$(CC) $(CFLAGS) -c proxy-sock.c -o proxy-sock.o

servidor-sock.o: servidor-sock.c
	$(CC) $(CFLAGS) -c servidor-sock.c -o servidor-sock.o

app-cliente-1.o: app-cliente-1.c
	$(CC) $(CFLAGS) -c app-cliente-1.c -o app-cliente-1.o

app-cliente-2.o: app-cliente-2.c
	$(CC) $(CFLAGS) -c app-cliente-2.c -o app-cliente-2.o

claves.o: claves.c
	$(CC) $(CFLAGS) -c claves.c -o claves.o

lines.o: lines.c
	$(CC) $(CFLAGS) -c lines.c -o lines.o

clean:
	rm -f *.o $(LIB_PROXY) $(SERVER_EXE) $(CLIENT1_EXE) $(CLIENT2_EXE)