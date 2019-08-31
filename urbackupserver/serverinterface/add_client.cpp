#include "action_header.h"
#include "../ClientMain.h"

ACTION_IMPL(add_client)
{
	Helper helper(tid, &POST, &PARAMS);
	JSON::Object ret;

	SUser *session = helper.getSession();
	if (session != NULL && session->id == SESSION_ID_INVALID) return;
	if (session != NULL && helper.getRights("add_client")=="all")
	{
		if (POST["clientname"].empty())
		{
			return;
		}

		int p_group_id = -1;
		str_map::iterator group_id = POST.find("group_id");
		if (group_id != POST.end())
		{
			p_group_id = watoi(group_id->second);
		}
		else
		{
			str_map::iterator group_name = POST.find("group_name");
			if (group_name != POST.end())
			{
				IQuery* q = helper.getDatabase()->Prepare("SELECT id FROM settings_db.si_client_groups WHERE name=?");
				q->Bind(group_name->second);
				db_results res = q->Read();
				q->Reset();

				if (!res.empty())
				{
					p_group_id = watoi(res[0]["id"]);
				}
			}
		}

		bool new_client = false;
		std::string new_authkey;
		int id = ClientMain::getClientID(helper.getDatabase(), POST["clientname"], NULL,
			&new_client, &new_authkey, p_group_id>0 ? &p_group_id : NULL);
		if (new_client)
		{
			ServerSettings settings(helper.getDatabase());

			SSettings* s = settings.getSettings();

			ret.set("new_clientid", id);
			ret.set("new_clientname", POST["clientname"]);
			ret.set("new_authkey", new_authkey);
			ret.set("internet_server", s->internet_server);
			ret.set("internet_server_port", s->internet_server_port);
			if (!s->internet_server_proxy.empty())
			{
				ret.set("internet_server_proxy", s->internet_server_proxy);
			}
			ret.set("added_new_client", true);
		}
		else
		{
			ret.set("already_exists", true);
		}
		ret.set("ok", true);
	}
	else
	{
		ret.set("error", 1);
	}
	helper.Write(ret.stringify(false));
}