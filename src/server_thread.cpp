class client_connection{
public:
	client_connection();
	~client_connection();
	send(char * msg; int num_bytes);
private:
	pthread_mutex_t client_mutex;

};