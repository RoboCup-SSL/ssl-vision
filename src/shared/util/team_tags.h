#ifndef TEAM_TAGS_H
#define TEAM_TAGS_H

#include <QObject>
#include <VarTypes.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>

class TeamTags;

class TeamTagPair : public QObject {
  Q_OBJECT
private:
  using VarType = VarTypes::VarType;

public:
  TeamTagPair();
  TeamTagPair(VarTypes::VarList *tag_pair);
  ~TeamTagPair() = default;

public slots:
  void tagChanged();

signals:
  void teamTagPairChanged(TeamTagPair*);
private:
  std::unique_ptr<VarTypes::VarList> tag_pair;
  std::unique_ptr<VarTypes::VarInt> tag_id;
  std::unique_ptr<VarTypes::VarTrigger> delete_button;

  friend TeamTags;
};

class TeamTags : public QObject {
  Q_OBJECT
private:
  using TagVarMap = std::unordered_map<int, std::unique_ptr<TeamTagPair>>;
  using TagSet = std::unordered_set<int>;
  using VarType = VarTypes::VarType;

public:
  TeamTags(const std::string &team_name);
  ~TeamTags() = default;

  VarTypes::VarList *getSettings() const { return settings.get(); }

  const TagSet &operator*() const { return tags; }

  const TagSet *operator->() const { return &tags; }

public slots:
  void tagListLoaded();
  void tagChanged(TeamTagPair* changed_tag_pair);
  void addTag();
  void deleteTag(VarType *deleted_var);

private:
  void sortSettingsList();
private:
  std::unique_ptr<VarTypes::VarList> settings;
  TagSet tags;
  TagVarMap tag_to_var_map;
  std::unique_ptr<VarTypes::VarTrigger> add_tag_trigger;
};

#endif /* TEAM_TAGS_H */
