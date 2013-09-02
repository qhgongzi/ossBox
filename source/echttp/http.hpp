#include "common.hpp"
#include "respone.hpp"
#include "request.hpp"
#include <boost/regex.hpp>
#include "client.hpp"
#include "iopool.hpp"
#include "detail/up_task.hpp"

namespace echttp
{
	class http
	{
	public:
		request Request;
		typedef	boost::function<void(boost::shared_ptr<respone>)> HttpCallBack;
        typedef	boost::function<void(int type,size_t total,size_t now)> StatusCallBack;

		http(void)
		{
			this->m_ioServ=&iopool::Instance(2)->io;
		}

		~http(void){}


        boost::shared_ptr<respone> Get(std::string url)
		{
			return this->_get("GET",url);
		}

        boost::shared_ptr<respone> Get(std::string url,std::string save_path)
		{
			return this->_get("GET",url,save_path);
		}


        void Get(std::string url,HttpCallBack cb)
		{
			this->_get("GET",url,cb);
			return ;
		}

        void Get(std::string url,std::string save_path,HttpCallBack cb)
		{
			this->_get("GET",url,save_path,cb);
			return ;
		}


        boost::shared_ptr<respone> Delete(std::string url)
		{
			return this->_get("DELETE",url);
		}


        void Delete(std::string url,HttpCallBack cb)
		{
            this->_get("DELETE",url,cb);
			return ;
		}


		boost::shared_ptr<respone> Post(std::string url,std::string data)
		{

			return this->_post("POST",url,data);
		}

		void Post(std::string url,std::string data,HttpCallBack cb)
		{
			this->_post("POST",url,data,cb);
			return ;
		}

        void Post(std::string url,std::vector<char> data,HttpCallBack cb)
		{
			this->_post("POST",url,data,cb);
			return ;
		}


		boost::shared_ptr<respone> Put(std::string url,std::string data)
		{
			return this->_post("PUT",url,data);
		}


		void Put(std::string url,std::string data,HttpCallBack cb)
		{
			this->_post("PUT",url,data,cb);
			return ;
		}


        void Put(std::string url,std::vector<char> data,HttpCallBack cb)
		{
			this->_post("PUT",url,data,cb);
			return ;
		}

        ///�첽PUT����put�����ݴ��ļ���ȡ
        // @url ����url
        // @file_path ���͵��ļ�·��
        // @cb �첽�ص�����
        // @status_cb д�룬��ȡ��״̬�ص���
        void PutFromFile(std::string url,std::string file_path,HttpCallBack cb)
		{
            this->_post_file("PUT",url,file_path,cb);
			return ;
		}

        ///ͬ��PUT����put�����ݴ��ļ���ȡ
        // @url ����url
        // @file_path ���͵��ļ�·��
        // @ status_cb д�룬��ȡ��״̬�ص���
        boost::shared_ptr<respone> PutFromFile(std::string url,std::string file_path)
		{
			return this->_post_file("PUT",url,file_path);
		}

        boost::shared_ptr<respone> PutToFile(std::string url,std::string data,std::string save_path)
		{
			return this->_post("PUT",url,data,save_path);
		}

        void PutToFile(std::string url,std::string data,std::string save_path,HttpCallBack cb)
		{
            this->_post("PUT",url,data,save_path,cb);
			return ;
		}

        ///�첽PUT���󣬲������д���ļ�
        // @url ����url
        // @data ���͵�vector<char>����
        // @save_path �������ݱ�����ļ�·��
        // @cb �첽�ص�����
        // @ status_cb д�룬��ȡ��״̬�ص���
        void PutToFile(std::string url,std::vector<char> data,std::string save_path,HttpCallBack cb)
		{
			this->_post("PUT",url,data,save_path,cb);
			return ;
		}


		void MessageBack(boost::shared_ptr<respone> result,HttpCallBack cb,client *client)
		{

			if(cb!=NULL)
			{
				cb(result);
			}
			if(client!=NULL)
			{
				delete client;
				client=NULL;
			}
		}

        void RegisterStatusCallBack(StatusCallBack cb)
        {
            this->m_status_callback=cb;
        }

		

	private:
		boost::asio::io_service *m_ioServ;
        StatusCallBack m_status_callback;


		void BuildHeader(boost::shared_ptr<respone> respone,std::string header)
		{
			if(header.find("HTTP")!=std::string::npos)
			{
				std::string h=header.substr(header.find(" ")+1);
				h=h.substr(0,h.find(" "));
				respone->status_code=convert<int,std::string>(h);

				boost::smatch result;
				std::string regtxt("\\b(\\w+?): (.*?)\r\n");
				boost::regex rx(regtxt);

				std::string::const_iterator it=header.begin();
				std::string::const_iterator end=header.end();

				while (regex_search(it,end,result,rx))
				{
					std::string key=result[1];
					std::string value=result[2];
					//respone->headerMap[key]=value;
					it=result[0].second;
				}
			}else
			{
				respone->status_code=-1;
			}
		}


        //����get����,delete֮���
        boost::shared_ptr<respone> _get(std::string method,std::string url)
		{

			up_task task=this->Request.make_task(method,url);

            boost::shared_ptr<respone> respone_(new respone());

            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> result=client.send();

			return result;

		}

        //get to filepath
        boost::shared_ptr<respone> _get(std::string method,std::string url,std::string save_path)
		{

			up_task task=this->Request.make_task(method,url);

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;

            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            } //ע���д״̬�ص�

			client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> result=client.send();

			return result;

		}


        void _get(std::string method,std::string url,HttpCallBack cb)
		{
			up_task task=this->Request.make_task(method,url);

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            boost::shared_ptr<client> httpClient(new client(*m_ioServ,task,respone_));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));
			return ;
		}

        void _get(std::string method,std::string url,std::string save_path,HttpCallBack cb)
		{
			up_task task=this->Request.make_task(method,url);

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

			boost::shared_ptr<client> httpClient(new client(*m_ioServ,task,respone_));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));
			return ;
		}



        //����post����
        boost::shared_ptr<respone> _post(std::string method,std::string url,std::string data)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> respone=client.send();

			return respone;
		}

        boost::shared_ptr<respone> _post(std::string method,std::string url,std::string data,std::string save_path)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> respone=client.send();

			return respone;
		}

        boost::shared_ptr<respone> _post_file(std::string method,std::string url,std::string file_path)
		{
			up_task  task=this->Request.make_file_task(method,url,std::vector<char>(file_path.begin(),file_path.end()));

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> respone=client.send();

			return respone;
		}

        boost::shared_ptr<respone> _post_file(std::string method,std::string url,std::string file_path,std::string save_path)
		{
			up_task  task=this->Request.make_file_task(method,url,std::vector<char>(file_path.begin(),file_path.end()));

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            client client(*m_ioServ,task,respone_);

			boost::shared_ptr<respone> respone=client.send();

			return respone;
		}


        void _post(std::string method,std::string url,std::string data,HttpCallBack cb)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            boost::shared_ptr<client> httpClient(new client(*m_ioServ,task,respone_));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post(std::string method,std::string url,std::string data,std::string save_path,HttpCallBack cb)
		{
			up_task  task=this->Request.make_task(method,url,std::vector<char>(data.begin(),data.end()));

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            boost::shared_ptr<client> httpClient(new client(*m_ioServ,task,respone_));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post_file(std::string method,std::string url,std::string file_path,HttpCallBack cb)
		{
			up_task  task=this->Request.make_file_task(method,url,std::vector<char>(file_path.begin(),file_path.end()));

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            boost::shared_ptr<client> httpClient(new client(*m_ioServ,task,respone_));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post_file(std::string method,std::string url,std::string file_path,std::string save_path,HttpCallBack cb)
		{
            up_task  task=this->Request.make_file_task(method,url,std::vector<char>(file_path.begin(),file_path.end()));

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }

            boost::shared_ptr<client> httpClient(new client(*m_ioServ,task,respone_));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post(std::string method,std::string url,std::vector<char> data,HttpCallBack cb)
		{
			up_task  task=this->Request.make_task(method,url,data);

            boost::shared_ptr<respone> respone_(new respone());
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }
            boost::shared_ptr<client> httpClient(new client(*m_ioServ,task,respone_));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}

        void _post(std::string method,std::string url,std::vector<char> data,std::string save_path,HttpCallBack cb)
		{
			up_task  task=this->Request.make_task(method,url,data);

            boost::shared_ptr<respone> respone_(new respone());
            respone_->save_path=save_path;
            if(m_status_callback)
            {
                respone_->register_notify_callback(m_status_callback);
                m_status_callback=0;
            }
            boost::shared_ptr<client> httpClient(new client(*m_ioServ,task,respone_));

			httpClient->send(boost::bind(&http::MessageBack,this,_1,cb,httpClient));

			return ;
		}


	};


}

