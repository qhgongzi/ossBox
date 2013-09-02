#pragma once
#include "common.hpp"
#include <boost/regex.hpp>

namespace echttp
{

class cookie_option
{
public:
	// ����option_item����.
	typedef std::pair<std::string, std::string> option_item;
	// ����option_item_list����.
	typedef std::vector<option_item> option_item_list;
	// for boost::assign::insert
	typedef option_item value_type;
public:
	cookie_option() {}
	~cookie_option() {}

public:

	// ���ѡ��, ��key/value��ʽ���.
	void insert(const std::string& key, const std::string& val)
	{
        remove(key);
		m_opts.push_back(option_item(key, val));
	}

	// ���ѡ��� std::part ��ʽ.
	void insert(value_type& item)
	{
        remove(item.first);
		m_opts.push_back(item);
	}

	// ɾ��ѡ��.
	cookie_option& remove(const std::string& key)
	{
		for (option_item_list::iterator i = m_opts.begin(); i != m_opts.end(); i++)
		{
			if (i->first == key)
			{
				m_opts.erase(i);
				return *this;
			}
		}
		return *this;
	}

	// ����ָ��key��value.
	bool find(const std::string& key, std::string& val) const
	{
		std::string s = key;
		boost::to_lower(s);
		for (option_item_list::const_iterator f = m_opts.begin(); f != m_opts.end(); f++)
		{
			std::string temp = f->first;
			boost::to_lower(temp);
			if (temp == s)
			{
				val = f->second;
				return true;
			}
		}
		return false;
	}

	// ����ָ���� key �� value. û�ҵ����� ""�������Ǹ�͵���İ���.
	std::string find(const std::string& key) const
	{
		std::string v;
		find(key,v);
		return v;
	}

	// �õ�cookie�ַ���.
	std::string cookie_string() const
	{
		std::string str;
		for (option_item_list::const_iterator f = m_opts.begin(); f != m_opts.end(); f++)
		{
			str += (f->first + "=" + f->second + "; ");
		}
		return str;
	}

    //��header�ַ��������滻cookie 
    void parse_header(const std::string &header)
    {
        boost::smatch result;
		std::string regtxt("Set-Cooki.*? (.*?)=(.*?);");
		boost::regex rx(regtxt);

		std::string::const_iterator it=header.begin();
		std::string::const_iterator end=header.end();

		while (regex_search(it,end,result,rx))
		{
			std::string cookie_key=result[1];
			std::string cookie_value=result[2];

            this->insert(cookie_key,cookie_value);

			it=result[0].second;
		}
    }

	// ���.
	void clear()
	{
		m_opts.clear();
	}

	// ��������option.
	option_item_list& option_all()
	{
		return m_opts;
	}

	// ���ص�ǰoption����.
	int size() const
	{
		return m_opts.size();
	}

protected:
	option_item_list m_opts;
};

}