#include "team_tags.h"
#include <string>

using namespace VarTypes;

TeamTagPair::TeamTagPair()
    : tag_pair{new VarList{"Unset"}}, tag_id{new VarInt{"Value", -1, -1}},
      delete_button{new VarTrigger{"Delete", "Delete"}} {
  tag_pair->addChild(tag_id.get());
  tag_pair->addChild(delete_button.get());

  connect(tag_id.get(), SIGNAL(wasEdited(VarType *)), this, SLOT(tagChanged()));
}

TeamTagPair::TeamTagPair(VarList *tag_pair) : tag_pair{tag_pair} {
  for (const auto &child : tag_pair->getChildren()) {
    if (VarInt *loaded_tag_id = dynamic_cast<VarInt *>(child)) {
      tag_id.reset(loaded_tag_id);
      connect(tag_id.get(), SIGNAL(wasEdited(VarType *)), this,
              SLOT(tagChanged()));
    } else if (VarTrigger *loaded_delete_button =
                   dynamic_cast<VarTrigger *>(child)) {
      loaded_delete_button->setLabel("Delete");
      delete_button.reset(loaded_delete_button);
    }
  }
}

void TeamTagPair::tagChanged() {
  const int value = tag_id->getInt();
  if (value >= 0) {
    tag_pair->setName(std::to_string(value));
  } else {
    tag_pair->setName("Unset");
  }

  emit teamTagPairChanged(this);
}

TeamTags::TeamTags(const std::string &team_name)
    : settings{new VarList{"" + team_name + " April Tags"}},
      add_tag_trigger{new VarTrigger{"Add Tag", "Add Tag"}} {

  settings->addChild(add_tag_trigger.get());
  connect(settings.get(), SIGNAL(XMLwasRead(VarType *)), this,
          SLOT(tagListLoaded()));
  connect(add_tag_trigger.get(), SIGNAL(wasEdited(VarType *)), this,
          SLOT(addTag()));
}

void TeamTags::tagListLoaded() {
  const auto children = settings->getChildren();

  tags.clear();
  tag_to_var_map.clear();

  for (const auto &child : children) {
    if (VarList *tag_pair = dynamic_cast<VarList *>(child)) {
      auto team_tag_pair =
          std::unique_ptr<TeamTagPair>(new TeamTagPair{tag_pair});
      const int id = team_tag_pair->tag_id->getInt();

      auto result = tags.insert(id);
      if (result.second) {
        connect(team_tag_pair.get(), SIGNAL(teamTagPairChanged(TeamTagPair *)),
                this, SLOT(tagChanged(TeamTagPair *)));
        connect(team_tag_pair->delete_button.get(),
                SIGNAL(hasChanged(VarType *)), this,
                SLOT(deleteTag(VarType *)));
        tag_to_var_map.insert({id, std::move(team_tag_pair)});
      } else {
        // this is a duplicate, remove from settings
        settings->removeChild(team_tag_pair->tag_pair.get());
      }
    }
  }

  sortSettingsList();
}

void TeamTags::tagChanged(TeamTagPair *changed_tag_pair) {
  // find the tag pair in the map
  int old_tag_id;
  std::unique_ptr<TeamTagPair> team_tag_pair;
  for (auto &tag_var_pair : tag_to_var_map) {
    if (tag_var_pair.second.get() == changed_tag_pair) {
      old_tag_id = tag_var_pair.first;
      team_tag_pair = std::move(tag_var_pair.second);
      break;
    }
  }

  if (!team_tag_pair) {
    return;
  }

  tags.erase(old_tag_id);
  tag_to_var_map.erase(old_tag_id);

  // try to insert it
  const int tag_id = team_tag_pair->tag_id->getInt();
  auto result = tags.insert(tag_id);
  if (result.second) {
    tag_to_var_map.insert({tag_id, std::move(team_tag_pair)});

    sortSettingsList();
  } else {
    settings->removeChild(team_tag_pair->tag_pair.get());
  }
}

void TeamTags::addTag() {
  // insert the new tag
  tags.insert(-1);
  auto result = tag_to_var_map.insert(
      {-1, std::unique_ptr<TeamTagPair>(new TeamTagPair{})});

  // if the insert failed then there is already an unset tag, so don't
  // bother connecting the new vartype, it will be delete after this
  // function exits
  if (result.second) {
    settings->addChild(result.first->second->tag_pair.get());
    connect(result.first->second.get(),
            SIGNAL(teamTagPairChanged(TeamTagPair *)), this,
            SLOT(tagChanged(TeamTagPair *)));
    connect(result.first->second->delete_button.get(),
            SIGNAL(hasChanged(VarType *)), this, SLOT(deleteTag(VarType *)));

    sortSettingsList();
  }
}

void TeamTags::deleteTag(VarType *deleted_var) {
  int tag_id;
  std::unique_ptr<TeamTagPair> team_tag_pair;
  for (auto &tag_var_pair : tag_to_var_map) {
    if (tag_var_pair.second->delete_button.get() == deleted_var) {
      tag_id = tag_var_pair.first;
      team_tag_pair = std::move(tag_var_pair.second);
    }
  }

  if (!team_tag_pair) {
    return;
  }

  settings->removeChild(team_tag_pair->tag_pair.get());
  tag_to_var_map.erase(tag_id);
  tags.erase(tag_id);
}

void TeamTags::sortSettingsList() {
  auto children = settings->getChildren();
  for (const auto &child : children) {
    settings->removeChild(child);
  }

  std::sort(children.begin(), children.end(),
            // should be no nullptr
            [](VarType *a, VarType *b) {
              assert(a != nullptr);
              assert(b != nullptr);

              if (VarList *tag_pair_a = dynamic_cast<VarList *>(a)) {
                if (VarList *tag_pair_b = dynamic_cast<VarList *>(b)) {
                  // find tag id var in pair a
                  int tag_id_a = -1;
                  for (const auto &child : tag_pair_a->getChildren()) {
                    if (VarInt *tag_id_var_a = dynamic_cast<VarInt *>(child)) {
                      tag_id_a = tag_id_var_a->getInt();
                    }
                  }
                  int tag_id_b = -1;
                  for (const auto &child : tag_pair_b->getChildren()) {
                    if (VarInt *tag_id_var_b = dynamic_cast<VarInt *>(child)) {
                      tag_id_b = tag_id_var_b->getInt();
                    }
                  }

                  return tag_id_a < tag_id_b;
                } else {
                  return false;
                }
              } else {
                // a isn't a list, so it is a button (or something else that
                // belongs at the top)
                if (VarList *tag_pair_b = dynamic_cast<VarList *>(b)) {
                  return true;
                } else {
                  return false;
                }
              }
            });

  for (auto &child : children) {
    settings->addChild(child);
  }
}
