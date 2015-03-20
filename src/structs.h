struct resourceHead
{
	//1=bitmap 2=sounds
	uint32_t dataType;
	uint32_t namesOffset;
	uint32_t indexOffset;
	uint32_t resourceCount;
}__attribute__((packed));

struct resourceData
{
	uint32_t resourceName;
	uint32_t resourceSize;
	uint32_t resourceDataOffset;
}__attribute__((packed));