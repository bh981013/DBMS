select t.name, sum(c.level)
from Trainer t, CatchedPokemon c
where t.id = c.owner_id
group by t.name
having sum(c.level)
order by sum(c.level) Desc, t.name limit 1;