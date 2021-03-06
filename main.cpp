#include <string>
#include <vector>
#include <unordered_set>
#include <set>
#include <memory>
#include <functional>
#include <unordered_map>
#include <exception>

template <class T>
struct BaseProperty_comp {
	typedef std::true_type is_transparent;

	struct Helper {
		std::string id;
		Helper() = default;
		Helper(Helper const&) = default;
		Helper(T *p) : id(p->id) {}
		template <class U, class... Ts>
		Helper(std::unique_ptr<U, Ts...> const &up) : id(up->id) {}
		bool operator<(Helper o) const {
			return std::less<std::string>() (id, o.id);
		}
	};

	bool operator()(Helper const &&lhs, Helper const &&rhs) const {
		return lhs < rhs;
	}
};

template <typename T>
using owning_set = std::set<std::unique_ptr<T>, BaseProperty_comp<T>>;

class BaseProperty {
	public:
		BaseProperty(const std::string &id) : id(id) {}
		virtual ~BaseProperty() {}

		bool operator==(const BaseProperty &p) const {
			return id == p.id;
		}
		
		std::string id;
};

template <typename T>
class Property : public BaseProperty {
	public:
		Property(const std::string &id, T&& data)
			: BaseProperty(id), value(std::move(data)) {}
		virtual ~Property() {}

		T value;
};

class Entity {
	public:
		using Functor = std::function<void(Entity*)>;
		Entity(const std::string &id, Functor registerP)
			: id(id), reg(registerP) {}

		template <typename T>
		void setProperty(const std::string &id, T&& data) {
			auto res = properties.emplace(id, std::move(data));
			if (!res.second)
				return;
			reg(this);
		}

		template <typename T>
		auto getProperty(const std::string &id) {
			auto res = properties.find(id);
			if (res != properties.end())
				return dynamic_cast<Property<T>*>(res->get());
			return nullptr;
		}

		std::string id;

	private:
		owning_set<BaseProperty> properties;
		Functor                  reg;
};

template <typename T>
auto &getProperty(Entity *e, const std::string &id) {
	auto *tmp = e->getProperty<T>(id);
	if (!tmp)
		throw std::runtime_error("Error property not present");
	return tmp->value;
}

class Requirements {
	public:
		template <typename T>
		void requireProperty(const std::string &id) {
			reqs.emplace([id](Entity *e) {
					return e->getProperty<T>(id) != nullptr;
					});
		}

		bool checkProperties(Entity *e) {
			for (auto &it : reqs)
				if (!it(e))
					return false;
			return true;
		}

	private:
		std::unordered_set<Entity::Functor> reqs;
};

class BaseController {
	public:
		BaseController(Entity *e) : entity(e) {}
		virtual ~BaseController() {}

		virtual void update() = 0;

	protected:
		Entity *entity;
};

class System {
	public:
		Entity *createEntity(const std::string &id) {
			auto res = entities.emplace(id,
					std::bind(&System::updateProperties,
						this, std::placeholders::_1));
			return res.first->get();
		}

		Entity *getEntity(const std::string &id) {
			auto res = entities.find(id);
			if (res != entities.end())
				return res->get();
			return nullptr;
		}

		void updateProperties(Entity *e) {
		}

		template <class T>
		void registerController() {
			auto reqs = T::require();
		}

		void update() {
		}

	private:
		using Functor =
			std::function<std::unique_ptr<BaseController>(Entity*)>;
		owning_set<Entity> entities;
		std::unordered_set<std::pair<Requirements, Functor>> controllers;
};

int main() {
	return 0;
}
