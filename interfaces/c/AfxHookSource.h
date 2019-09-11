#ifndef ADVANCEDFX_AFXHOOKSOURCE_H
#define ADVANCEDFX_AFXHOOKSOURCE_H




#define ADVANCEDFX_LOCALEID_EN 0

typedef int advancedfx_LocaleID;


#define ADVANCEDFX_LIBRARY_INTERFACE_VERSION_1 1

typedef int advancedfx_LibraryInterfaceVersion;

struct advancedfx_ILibrary {

	advancedfx_LibraryInterfaceVersion(*AfxHookSourceVersion)();

	/**
	 * @param afxHookSource IAfxHookSource_xxx according to LibraryInterfaceVersion.
	 * @returns IAssembly_xxx according to advancedfx_LibraryInterfaceVersion.
	 */
	void *(*Construct)(void * afxHookSource);
};

#define ADVANCEDFX_OBJECTTYPE_IOBJECT 0

typedef int advancedfx_ObjectType;


#define ADVANCEDFX_JITTYPE_LOADLIBARY 0
#define ADVANCEDFX_JITTYPE_PYTHON 1

typedef int advancedfx_JitType;


typedef int advancedfx_EntityHandle;

typedef struct advancedfx_QVector {
	float x;
	float y;
	float z;
};

typedef struct advancedfx_QAngles {
	float x;
	float y;
	float z;
};


struct advancedfx_IString {
	void(*AddRef)(advancedfx_IString * This);
	void(*Release)(advancedfx_IString * This);
	char const *(*Get)(advancedfx_IString * This);
};

struct advancedfx_IDependents;

/**
 * IObject shall delete itself when it's removed from the Dependents of a compulsory IObject.
 * IAfxHookSource has root IObject without dependencies, all other IObjects depend on it.
 * There must be no circles in the dependency graph!
 */
struct advancedfx_IObject {

	/** Request explicit deletion of the object.
	 * @returns true if deleted, false if request was rejected.
	 */
	bool(*Delete)(advancedfx_IObject * This);

	advancedfx_ObjectType(*Type)(advancedfx_IObject * This);
	
	/**
	 * @returns Pointer to the Object of type IObject.Type.
	 */
	void *(*Base)(advancedfx_IObject * This);
	
	advancedfx_IDependents *(*Dependents)(advancedfx_IObject * This);
	
	advancedfx_IString * (*Name)(advancedfx_IObject * This);
};


struct advancedfx_IDependentsNode {
	advancedfx_IObject IObject;

	advancedfx_IDependentsNode *(*Next)(advancedfx_IDependentsNode * This);

	advancedfx_IObject *(*Value)(advancedfx_IDependentsNode * This);
};

struct advancedfx_IDependentsNotifyee {
	advancedfx_IObject IObject;

	void(*Notify)(advancedfx_IDependentsNotifyee * This, advancedfx_IDependents * value);
};

struct advancedfx_IDependents {
	advancedfx_IDependentsNode *(*Begin)(advancedfx_IDependents * This);
	void(*SetBegin)(advancedfx_IDependents * This, advancedfx_IDependentsNode * value);
	void(*AddBeginChanged)(advancedfx_IDependents * This, advancedfx_IDependentsNotifyee * notifyee);
};

struct advancedfx_IAssembly_1
{
	advancedfx_IObject IObject;
};

struct advancedfx_IJit {
	advancedfx_IObject IObject;

	int(*Type)(advancedfx_IJit * This);

	/**
	 * @returns 0 upon error, assembly otherwise.
	 */
	advancedfx_IAssembly_1 *(*Execute)(advancedfx_IJit * This, char const * code, advancedfx_IString ** outErrorString /* can be 0 */);
};

struct advancedfx_IJitsNode {
	advancedfx_IObject IObject;

	advancedfx_IJitsNode *(*Next)(advancedfx_IJitsNode * This);

	advancedfx_IJit *(*Value)(advancedfx_IJitsNode * This);
};

struct advancedfx_IJitsNotifyee {
	advancedfx_IObject IObject;

	void(*Notify)(advancedfx_IJitsNotifyee * This, advancedfx_IJits * value);
};

struct advancedfx_IJits {
	advancedfx_IJitsNode *(*Begin)(advancedfx_IJits * This);
	void(*SetBegin)(advancedfx_IJits * This, advancedfx_IJitsNode * value);
	void(*AddBeginChanged)(advancedfx_IJits * This, advancedfx_IJitsNotifyee * notifyee);
};


struct advancedfx_IEntity {
	advancedfx_IObject IObject;

	advancedfx_EntityHandle(*GetHandle)(advancedfx_IEntity * This);

	int(*GetEntIndex)(advancedfx_IEntity * This);

	advancedfx_IEntity *(*GetActiveWeapon)(advancedfx_IEntity * This);

	void(*GetAbsOrigin)(advancedfx_IEntity * This, advancedfx_QVector * outValue);

	void(*GetAbsAngles)(advancedfx_IEntity * This, advancedfx_QAngles * outValue);

	void(*EyePosition)(advancedfx_IEntity * This, advancedfx_QVector * outValue);

	void(*EyeAngles)(advancedfx_IEntity * This, advancedfx_QAngles * outValue);

	int(*LookUpAttachment)(advancedfx_IEntity * This, const char * attachmentName);

	bool(*GetAttachment)(advancedfx_IEntity * This, int number, advancedfx_QVector * outOrigin, advancedfx_QAngles * outAngles);
};

struct advancedfx_IEntitiesNode {
	advancedfx_IObject IObject;

	advancedfx_IEntitiesNode *(*Next)(advancedfx_IEntitiesNode * This);

	advancedfx_IEntity *(*Value)(advancedfx_IEntitiesNode * This);
};

struct advancedfx_IEntitiesNotifyee {
	advancedfx_IObject IObject;

	void(*Notify)(advancedfx_IEntitiesNotifyee * This, advancedfx_IEntities * value);
};

struct advancedfx_IEntities {
	advancedfx_IEntitiesNode *(*Begin)(advancedfx_IEntities * This);
	void(*SetBegin)(advancedfx_IEntities * This, advancedfx_IEntitiesNode * value);
	void(*AddBeginChanged)(advancedfx_IEntities * This, advancedfx_IEntitiesNotifyee * notifyee);
};


struct advancedfx_ISetupEngineViewHandler {
	advancedfx_IObject IObject;

	bool(* Handle)(bool lastResult, advancedfx_QVector * inOutOrigin, advancedfx_QAngles * inOutAngles, float * inOutfov);
};

struct advancedfx_ISetupEngineViewHandlersNode {
	advancedfx_IObject IObject;

	advancedfx_ISetupEngineViewHandlersNode *(*Next)(advancedfx_ISetupEngineViewHandlersNode * This);

	advancedfx_ISetupEngineViewHandler *(*Value)(advancedfx_ISetupEngineViewHandlersNode * This);
};

struct advancedfx_ISetupEngineViewHandlersNotifyee {
	advancedfx_IObject IObject;

	void(*Notify)(advancedfx_ISetupEngineViewHandlersNotifyee * This, advancedfx_ISetupEngineViewHandlers * value);
};

struct advancedfx_ISetupEngineViewHandlers {
	advancedfx_ISetupEngineViewHandlersNode *(*Begin)(advancedfx_ISetupEngineViewHandlers * This);
	void(*SetBegin)(advancedfx_ISetupEngineViewHandlers * This, advancedfx_ISetupEngineViewHandlersNode * value);
	void(*AddBeginChanged)(advancedfx_ISetupEngineViewHandlers * This, advancedfx_ISetupEngineViewHandlersNotifyee * notifyee);
};


struct advancedfx_IAfxHookSource_1 {
	advancedfx_IObject IObject;


	// Tools:

	advancedfx_LocaleID(*LocaleID)(advancedfx_IAfxHookSource_1 * This);


	// SetupEngineViewHandler:

	advancedfx_ISetupEngineViewHandlersNode * (*SetupEngineViewHandlers)(advancedfx_IAfxHookSource_1 * This);


	// Entities:

	advancedfx_IEntities *(*Entities)(advancedfx_IAfxHookSource_1 * This);

	advancedfx_IEntity *(*EntityFromHandle)(advancedfx_IAfxHookSource_1 * This, advancedfx_EntityHandle handle);

	advancedfx_IEntity *(*EntityFromIndex)(advancedfx_IAfxHookSource_1 * This, int index);

	advancedfx_IEntity *(*EntityFromSpecKey)(advancedfx_IAfxHookSource_1 * This, int keyNumber);


	// Jits

	advancedfx_IJits *(*Jits)(advancedfx_IAfxHookSource_1 * This);
};


#endif
