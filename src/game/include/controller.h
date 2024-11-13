#include"object.h"
#include<memory.h>
class Controller :public Object
{
protected:
	std::weak_ptr<SceneObject> m_owner;
public:
	Controller(std::shared_ptr<World> world);
	void SetOwner(std::shared_ptr<SceneObject> owner);
	virtual void tick();
};