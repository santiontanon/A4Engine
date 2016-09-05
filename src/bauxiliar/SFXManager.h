#ifndef _SFX_MANAGER
#define _SFX_MANAGER


class SFXManagerNode {
public:
	~SFXManagerNode();

	Mix_Chunk *m_sfx;
	Symbol *m_name;
};


class SFXManager {
public:
	SFXManager();
	~SFXManager();

	Mix_Chunk *get(const char *name);
	Mix_Chunk *get(Symbol *name);

	void next_cycle(void);	// clears the list of already played SFX
	void cache(const char *dirname);

protected:

	std::vector<SFXManagerNode *> *m_hash;

	std::vector<SFXManagerNode *> m_already_played;
	char *remove_extension_copy(const char *filename);
};

#endif


