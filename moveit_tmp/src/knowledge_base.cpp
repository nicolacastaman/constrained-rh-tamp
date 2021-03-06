#include <moveit_tmp/knowledge_base.h>

namespace moveit_tmp
{
KnowledgeBase::KnowledgeBase()
{
  on_ = std::make_shared<On>(this);
  in_hand_ = std::make_shared<InHand>(this);
  obstruct_ = std::make_shared<Obstruct>(this);
  is_stackable_ = std::make_shared<IsStackable>(this);
  leave_clean_ = std::make_shared<LeaveClean>(this);
  loaded_ = std::make_shared<Loaded>(this);
  near_ = std::make_shared<Near>(this);
  located_in_ = std::make_shared<LocatedIn>(this);
}

KnowledgeBase::KnowledgeBase(const KnowledgeBase& kb)
    : objects_(kb.objects_), on_(std::make_shared<On>(*kb.on_)),
      in_hand_(std::make_shared<InHand>(*kb.in_hand_)),
      obstruct_(std::make_shared<Obstruct>(*kb.obstruct_)),
      is_stackable_(std::make_shared<IsStackable>(*kb.is_stackable_)),
      leave_clean_(std::make_shared<LeaveClean>(*kb.leave_clean_)),
      loaded_(std::make_shared<Loaded>(*kb.loaded_)),
      near_(std::make_shared<Near>(*kb.near_)),
      located_in_(std::make_shared<LocatedIn>(*kb.located_in_))
{
}

std::string KnowledgeBase::getPDDL()
{
  return getObjectsPDDL(true) + getInitPDDL(true);
}

std::string KnowledgeBase::getObjectsPDDL(bool header)
{
  std::string pddl;

  if (header)
    pddl += "(:objects";

  std::string movable_;
  std::string fixed_;
  std::string surface_;
  std::string place_;

  for (auto const& object : objects_)
  {
    if (object.type == MOVABLE)
    {
      movable_ += " " + object.id;
    }
    else if (object.type == FIXED)
    {
      fixed_ += " " + object.id;
    }
    else if (object.type == SURFACE)
    {
      surface_ += " " + object.id;
    }
    else if (object.type == PLACE)
    {
      place_ += " " + object.id;
    }
  }

  if (movable_ != "")
    pddl += movable_ + " - movable";

  if (fixed_ != "")
    pddl += fixed_ + " - fixed";

  if (surface_ != "")
    pddl += surface_ + " - surface";

  if (place_ != "")
    pddl += place_ + " - place";

  if (header)
    pddl += ")";

  return pddl;
}

std::string KnowledgeBase::getInitPDDL(bool header)
{
  std::string pddl;

  if (header)
    pddl += "(:init";

  pddl += obstruct_->getPDDL();

  pddl += leave_clean_->getPDDL();

  pddl += on_->getPDDL();

  pddl += is_stackable_->getPDDL();

  pddl += in_hand_->getPDDL();

  pddl += loaded_->getPDDL();

  pddl += near_->getPDDL();

  pddl += located_in_->getPDDL();

  for (auto const& object : objects_)
  {
    if (object.type == MOVABLE)
      pddl += "(= (weight " + object.id + ") " + std::to_string(object.weight) +
              ")";
  }

  if (header)
    pddl += ")";

  return pddl;
}

void KnowledgeBase::addObject(const Object& object) { objects_.insert(object); }

void KnowledgeBase::addObject(const std::string object_id,
                              const ObjectType object_type)
{
  Object object = {object_id, object_type, false};
  addObject(object);
}

bool KnowledgeBase::getObjectById(const std::string object_id, Object& object)
{
  auto it = findObjectById(object_id);

  if (it == objects_.end())
    return false;

  object = *it;

  return true;
}

std::set<Object, ObjectComparator>::iterator
KnowledgeBase::findObjectById(const std::string object_id)
{
  /*
return std::find_if(
    objects_.begin(), objects_.end(),
    [object_id](const Object& o) -> bool { return o.id == object_id; });
    */

  auto it = std::find_if(
      objects_.begin(), objects_.end(),
      [object_id](const auto& o) -> bool { return o.id == object_id; });

  return it;
}

void KnowledgeBase::removeObject(const std::string object_id)
{
  if (findObjectById(object_id) == objects_.end())
    return;

  on_->removeObject(object_id);
  in_hand_->removeObject(object_id);
  obstruct_->removeObject(object_id);
  is_stackable_->removeObject(object_id);
  leave_clean_->removeObject(object_id);
  loaded_->removeObject(object_id);
  near_->removeObject(object_id);
  located_in_->removeObject(object_id);

  objects_.erase(findObjectById(object_id));
}

void KnowledgeBase::reset() {}

std::shared_ptr<KnowledgeBase::On> KnowledgeBase::getOn() { return on_; }

std::shared_ptr<KnowledgeBase::InHand> KnowledgeBase::getInHand()
{
  return in_hand_;
}

std::shared_ptr<KnowledgeBase::Obstruct> KnowledgeBase::getObstruct()
{
  return obstruct_;
}

std::shared_ptr<KnowledgeBase::IsStackable> KnowledgeBase::getIsStackable()
{
  return is_stackable_;
}

std::shared_ptr<KnowledgeBase::LeaveClean> KnowledgeBase::getLeaveClean()
{
  return leave_clean_;
}

std::shared_ptr<KnowledgeBase::Loaded> KnowledgeBase::getLoaded()
{
  return loaded_;
}

std::shared_ptr<KnowledgeBase::Near> KnowledgeBase::getNear() { return near_; }

std::shared_ptr<KnowledgeBase::LocatedIn> KnowledgeBase::getLocatedIn()
{
  return located_in_;
}

// On Fact //

void KnowledgeBase::On::add(const std::string object_1,
                            const std::string object_2)
{
  // TODO evitare un oggetto sopra a 2 oggetti
  removeObject(object_1); // replace the object knowledge
  on_.push_back(std::make_pair(object_1, object_2));
}

bool KnowledgeBase::On::getUnderById(const std::string object_id,
                                     Object& on_object)
{

  auto it = std::find_if(
      on_.begin(), on_.end(),
      [object_id](const std::pair<std::string, std::string>& on) -> bool {
        return on.first == object_id;
      });

  if (it != on_.end())
  {
    kb_->getObjectById(it->second, on_object);
    return true;
  }

  return false;
}

void KnowledgeBase::On::removeObject(const std::string object_id)
{
  for (auto it = on_.begin(); it != on_.end(); ++it)
  {
    if (it->first == object_id || it->second == object_id)
    {
      on_.erase(it--); // decrese iterator to match new vector size
    }
  }
}

std::string KnowledgeBase::On::getPDDL()
{
  std::string pddl;

  for (auto const& on : on_)
  {
    pddl += "(on " + on.first + " " + on.second + ")";
  }

  return pddl;
}

void KnowledgeBase::On::reset() { on_.clear(); }

// Obstruct Fact //

void KnowledgeBase::Obstruct::add(std::string obstruction,
                                  std::string obstructed)
{
  std::map<std::string, std::vector<std::string>>::iterator constraints_it =
      obstructions_.find(obstruction);
  if (constraints_it != obstructions_.end())
  {
    constraints_it->second.push_back(obstructed);
  }
  else
  {
    obstructions_[obstruction] = {obstructed};
  }
}

void KnowledgeBase::Obstruct::removeObject(std::string object_id)
{
  obstructions_.erase(object_id);

  for (auto& relation : obstructions_)
  {
    relation.second.erase(
        std::remove(relation.second.begin(), relation.second.end(), object_id),
        relation.second.end());
  }
}

std::string KnowledgeBase::Obstruct::getPDDL()
{
  std::string pddl;

  for (auto const& obstruct : obstructions_)
  {
    for (auto const& obstructed : obstruct.second)
    {
      pddl += "(obstruct " + obstruct.first + " " + obstructed + ")";
    }
  }

  return pddl;
}

void KnowledgeBase::Obstruct::reset() { obstructions_.clear(); }

// InHand Fact //

void KnowledgeBase::InHand::add(std::string object_id) { in_hand_ = object_id; }

void KnowledgeBase::InHand::empty() { in_hand_ = ""; }

void KnowledgeBase::InHand::removeObject(std::string object_id)
{
  if (in_hand_ == object_id)
    empty();
}

std::string KnowledgeBase::InHand::getPDDL()
{
  std::string pddl;

  if (in_hand_ != "")
    pddl += "(not (arm-empty))(holding " + in_hand_ + ")";
  else
    pddl += "(arm-empty)";

  return pddl;
}

void KnowledgeBase::InHand::reset() { in_hand_.clear(); }

// Is Stackable Fact //

void KnowledgeBase::IsStackable::add(std::string object_id, bool stackable)
{
  // TODO check if exist
  removeObject(object_id);
  stackables_.push_back(std::make_pair(object_id, stackable));
}

std::string KnowledgeBase::IsStackable::getPDDL()
{

  std::string pddl;

  for (auto const& surface : stackables_)
  {
    std::string predicate = "(is-stackable " + surface.first + ")";

    if (!surface.second)
      pddl += "(not " + predicate + ")";
    else
      pddl += predicate;
  }

  return pddl;
}

void KnowledgeBase::IsStackable::removeObject(std::string object_id)
{
  for (auto it = stackables_.begin(); it != stackables_.end(); ++it)
  {
    if (it->first == object_id)
    {
      stackables_.erase(it--); // decrese iterator to match new vector size
    }
  }
}

void KnowledgeBase::IsStackable::reset() { stackables_.clear(); }

// LeaveClean Fact //

void KnowledgeBase::LeaveClean::add(std::string object_1, std::string object_2)
{
  // TODO evitare un oggetto sopra a 2 oggetti
  removeObject(object_1); // replace the object knowledge
  leave_clean_.push_back(std::make_pair(object_1, object_2));
}

void KnowledgeBase::LeaveClean::removeObject(std::string object_id)
{
  for (auto it = leave_clean_.begin(); it != leave_clean_.end(); ++it)
  {
    if (it->first == object_id || it->second == object_id)
    {
      leave_clean_.erase(it--); // decrese iterator to match new vector size
    }
  }
}

std::string KnowledgeBase::LeaveClean::getPDDL()
{
  std::string pddl;

  for (auto const& leave_clean : leave_clean_)
  {
    pddl += "(leave-free " + leave_clean.first + " " + leave_clean.second + ")";
  }

  return pddl;
}

void KnowledgeBase::LeaveClean::reset() { leave_clean_.clear(); }

// Loaded Fact //

void KnowledgeBase::Loaded::add(std::string object_id)
{
  loaded_.insert(object_id);
}

std::string KnowledgeBase::Loaded::getPDDL()
{
  std::string pddl;

  for (auto const& loaded : loaded_)
  {
    if (loaded != "") // TODO improve
      pddl += "(loaded " + loaded + ")";
  }
  return pddl;
}

void KnowledgeBase::Loaded::removeObject(std::string object_id)
{
  //std::remove(loaded_.begin(), loaded_.end(), object_id);
  loaded_.erase(object_id);
}

void KnowledgeBase::Loaded::reset() { loaded_.clear(); }

// Near Fact //

void KnowledgeBase::Near::add(std::string object_id) { near_ = object_id; }

void KnowledgeBase::Near::empty() { near_ = ""; }

void KnowledgeBase::Near::removeObject(std::string object_id)
{
  if (near_ == object_id)
    empty();
}

std::string KnowledgeBase::Near::getPDDL()
{
  std::string pddl;

  if (near_ != "")
    pddl += "(near " + near_ + ")";

  return pddl;
}

void KnowledgeBase::Near::reset() { near_.clear(); }

// LocatedIn Fact //

void KnowledgeBase::LocatedIn::add(const std::string object_1,
                                   const std::string object_2)
{
  // TODO evitare un oggetto sopra a 2 oggetti
  removeObject(object_1); // replace the object knowledge
  located_in_.push_back(std::make_pair(object_1, object_2));
}

std::string
KnowledgeBase::LocatedIn::getLocationById(const std::string object_id)
{

  auto it = std::find_if(
      located_in_.begin(), located_in_.end(),
      [object_id](const std::pair<std::string, std::string>& in) -> bool {
        return in.first == object_id;
      });

  if (it != located_in_.end())
  {
    return it->second;
  }

  return "";
}

void KnowledgeBase::LocatedIn::removeObject(const std::string object_id)
{
  for (auto it = located_in_.begin(); it != located_in_.end(); ++it)
  {
    if (it->first == object_id || it->second == object_id)
    {
      located_in_.erase(it--); // decrese iterator to match new vector size
    }
  }
}

std::string KnowledgeBase::LocatedIn::getPDDL()
{
  std::string pddl;

  for (auto const& located_in : located_in_)
  {
    pddl += "(located-in " + located_in.first + " " + located_in.second + ")";
  }

  return pddl;
}

void KnowledgeBase::LocatedIn::reset() { located_in_.clear(); }

} // namespace moveit_tmp
